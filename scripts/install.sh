#!/usr/bin/env bash
# Install the latest BennuGD64 release into $HOME/bennugd and configure the shell.
# Usage:
#   curl -sL https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.sh | bash
set -euo pipefail

REPO="${BENNUGD_REPO:-humbertodias/BennuGD64}"
INSTALL_DIR="${BENNUGD_HOME:-$HOME/bennugd}"
VERSION="${BENNUGD_VERSION:-latest}"
LINKAGE="${BENNUGD_LINKAGE:-static}"
GITHUB_API="${GITHUB_API:-https://api.github.com}"
GITHUB_DOWNLOAD="${GITHUB_DOWNLOAD:-https://github.com}"

bold() { printf '\033[1m%s\033[0m\n' "$*"; }
info() { printf '  %s\n' "$*"; }
die()  { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

detect_platform() {
  local os arch
  os="$(uname -s | tr '[:upper:]' '[:lower:]')"
  arch="$(uname -m)"

  case "$os" in
    linux*)  PLATFORM=linux ;;
    darwin*) PLATFORM=macos ;;
    msys*|mingw*|cygwin*) PLATFORM=windows ;;
    *) die "Unsupported OS: $(uname -s)" ;;
  esac

  case "$arch" in
    x86_64|amd64) ARCH=x86_64 ;;
    arm64|aarch64) ARCH=arm64 ;;
    *) die "Unsupported architecture: $arch" ;;
  esac
}

resolve_version() {
  if [[ "$VERSION" != "latest" ]]; then
    TAG="$VERSION"
    return
  fi

  need_cmd curl
  local json
  json="$(curl -fsSL "$GITHUB_API/repos/$REPO/releases/latest")" || \
    die "Failed to query GitHub releases for $REPO"

  TAG="$(printf '%s\n' "$json" | sed -n 's/.*"tag_name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | head -n1)"
  [[ -n "$TAG" ]] || die "Could not determine latest release tag"
}

asset_name() {
  local suffix="${1:-}"
  if [[ "$PLATFORM" == "windows" ]]; then
    if [[ -n "$suffix" ]]; then
      printf 'bennugd64-%s-%s-%s-%s.zip' "$TAG" "$PLATFORM" "$ARCH" "$suffix"
    else
      printf 'bennugd64-%s-%s-%s.zip' "$TAG" "$PLATFORM" "$ARCH"
    fi
  else
    if [[ -n "$suffix" ]]; then
      printf 'bennugd64-%s-%s-%s-%s.tar.gz' "$TAG" "$PLATFORM" "$ARCH" "$suffix"
    else
      printf 'bennugd64-%s-%s-%s.tar.gz' "$TAG" "$PLATFORM" "$ARCH"
    fi
  fi
}

download_and_extract() {
  local asset url stage extracted
  case "$LINKAGE" in
    static|shared) ;;
    *) die "BENNUGD_LINKAGE must be static or shared (got: $LINKAGE)" ;;
  esac
  asset="$(asset_name "$LINKAGE")"
  url="$GITHUB_DOWNLOAD/$REPO/releases/download/$TAG/$asset"
  WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/bennugd-install.XXXXXX")"
  cleanup() { rm -rf "${WORK_DIR:-}"; }
  trap cleanup EXIT

  info "Downloading $asset"
  if ! curl -fL --progress-bar -o "$WORK_DIR/$asset" "$url"; then
    # Older releases used unsuffixed archive names.
    asset="$(asset_name)"
    url="$GITHUB_DOWNLOAD/$REPO/releases/download/$TAG/$asset"
    info "Retrying legacy asset $asset"
    curl -fL --progress-bar -o "$WORK_DIR/$asset" "$url" || \
      die "Download failed (no asset for $PLATFORM/$ARCH/$LINKAGE at $TAG?): $url"
  fi

  stage="$WORK_DIR/extract"
  mkdir -p "$stage"

  if [[ "$PLATFORM" == "windows" ]]; then
    if command -v unzip >/dev/null 2>&1; then
      unzip -q "$WORK_DIR/$asset" -d "$stage"
    elif command -v powershell.exe >/dev/null 2>&1; then
      powershell.exe -NoProfile -Command \
        "Expand-Archive -LiteralPath '$WORK_DIR/$asset' -DestinationPath '$stage' -Force"
    else
      die "Need 'unzip' (or powershell.exe) to extract Windows zip"
    fi
  else
    need_cmd tar
    tar -xzf "$WORK_DIR/$asset" -C "$stage"
  fi

  extracted="$(find "$stage" -mindepth 1 -maxdepth 1 -type d | head -n1)"
  [[ -n "$extracted" ]] || die "Archive did not contain an install directory"

  rm -rf "$INSTALL_DIR"
  mkdir -p "$(dirname "$INSTALL_DIR")"
  mv "$extracted" "$INSTALL_DIR"

  if [[ "$PLATFORM" != "windows" ]]; then
    chmod +x "$INSTALL_DIR/bgdc" "$INSTALL_DIR/bgdi" 2>/dev/null || true
  fi

  cleanup
  trap - EXIT
}

profile_file() {
  local shell_name
  shell_name="$(basename "${SHELL:-bash}")"
  case "$shell_name" in
    zsh)  printf '%s\n' "${ZDOTDIR:-$HOME}/.zshrc" ;;
    bash)
      if [[ -f "$HOME/.bashrc" ]]; then
        printf '%s\n' "$HOME/.bashrc"
      elif [[ -f "$HOME/.bash_profile" ]]; then
        printf '%s\n' "$HOME/.bash_profile"
      else
        printf '%s\n' "$HOME/.profile"
      fi
      ;;
    fish) printf '%s\n' "$HOME/.config/fish/config.fish" ;;
    *)    printf '%s\n' "$HOME/.profile" ;;
  esac
}

write_env_block() {
  local profile marker_begin marker_end block
  profile="$(profile_file)"
  marker_begin="# >>> bennugd64 >>>"
  marker_end="# <<< bennugd64 <<<"

  mkdir -p "$(dirname "$profile")"
  touch "$profile"

  if [[ "$(basename "${SHELL:-}")" == "fish" ]]; then
    block=$(cat <<EOF
$marker_begin
set -gx BENNUGD_HOME "$INSTALL_DIR"
if not contains \$BENNUGD_HOME \$PATH
  set -gx PATH \$BENNUGD_HOME \$PATH
end
$marker_end
EOF
)
  else
    block=$(cat <<EOF
$marker_begin
export BENNUGD_HOME="$INSTALL_DIR"
export PATH="\$BENNUGD_HOME:\$PATH"
$marker_end
EOF
)
  fi

  if grep -qF "$marker_begin" "$profile" 2>/dev/null; then
    # macOS awk rejects newlines in -v; strip the old block, then append.
    local tmp_profile
    tmp_profile="$(mktemp)"
    awk -v begin="$marker_begin" -v end="$marker_end" '
      $0 == begin { skip=1; next }
      $0 == end { skip=0; next }
      !skip { print }
    ' "$profile" > "$tmp_profile"
    printf '\n%s\n' "$block" >> "$tmp_profile"
    mv "$tmp_profile" "$profile"
  else
    printf '\n%s\n' "$block" >> "$profile"
  fi

  PROFILE_UPDATED="$profile"
}

persist_windows_user_env() {
  # Also persist for native Windows sessions when running under Git Bash / MSYS.
  command -v powershell.exe >/dev/null 2>&1 || return 0

  local win_home
  win_home="$(cygpath -w "$INSTALL_DIR" 2>/dev/null || printf '%s' "$INSTALL_DIR")"

  powershell.exe -NoProfile -Command "
    \$homePath = '$win_home'
    [Environment]::SetEnvironmentVariable('BENNUGD_HOME', \$homePath, 'User')
    \$path = [Environment]::GetEnvironmentVariable('Path', 'User')
    if (-not \$path) { \$path = '' }
    \$parts = @(\$path -split ';' | Where-Object { \$_ -and \$_ -ne \$homePath })
    \$newPath = (,@(\$homePath) + \$parts) -join ';'
    [Environment]::SetEnvironmentVariable('Path', \$newPath, 'User')
  " >/dev/null || true
}

main() {
  bold "BennuGD64 installer"
  need_cmd curl
  detect_platform
  resolve_version

  info "Version : $TAG"
  info "Platform: $PLATFORM/$ARCH"
  info "Linkage : $LINKAGE"
  info "Install : $INSTALL_DIR"
  echo

  download_and_extract
  write_env_block
  if [[ "$PLATFORM" == "windows" ]]; then
    persist_windows_user_env
  fi

  # Make current shell session usable immediately when not piped oddly.
  export BENNUGD_HOME="$INSTALL_DIR"
  export PATH="$BENNUGD_HOME:$PATH"

  echo
  bold "Installed successfully."
  info "BENNUGD_HOME=$INSTALL_DIR"
  info "Updated shell config: $PROFILE_UPDATED"
  info "Open a new terminal, or run: source \"$PROFILE_UPDATED\""
  info "Then try: bgdc -help && bgdi -help"
}

main "$@"
