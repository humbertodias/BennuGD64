/* Minimal SAL annotations for building SDL3 with nxdk (no MSVC sal.h).
 * Macros must accept arguments and expand to nothing so they can appear
 * before a parameter type: `_Out_bytecap_(n) void *p`.
 */
#ifndef __BENNUGD_SAL_H
#define __BENNUGD_SAL_H

#define _In_
#define _Out_
#define _Inout_
#define _In_opt_
#define _Out_opt_
#define _Inout_opt_
#define _Outptr_
#define _Outptr_opt_
#define _Outptr_result_maybenull_
#define _Ret_maybenull_
#define _Ret_notnull_
#define _Check_return_
#define _Success_(x)
#define _Printf_format_string_
#define _Scanf_format_string_
#define _Scanf_format_string_impl_
#define _Null_terminated_
#define _Post_invalid_
#define _Use_decl_annotations_
#define _Analysis_assume_(x)
#define _When_(a, b)

#define _In_z_
#define _In_opt_z_
#define _Out_z_cap_(x)
#define _Out_writes_(x)
#define _Out_writes_opt_(x)
#define _Out_writes_z_(x)
#define _Out_writes_bytes_(x)
#define _Out_writes_bytes_opt_(x)
#define _Out_writes_bytes_to_(a, b)
#define _Out_writes_bytes_to_opt_(a, b)
#define _Out_writes_to_(a, b)
#define _Out_writes_to_opt_(a, b)
#define _In_reads_(x)
#define _In_reads_opt_(x)
#define _In_reads_z_(x)
#define _In_reads_or_z_(x)
#define _In_reads_bytes_(x)
#define _In_reads_bytes_opt_(x)
#define _Inout_updates_(x)
#define _Inout_updates_opt_(x)
#define _Inout_updates_z_(x)
#define _Inout_updates_bytes_(x)
#define _Inout_z_cap_(x)
#define _Out_bytecap_(x)
#define _Out_opt_bytecap_(x)
#define _In_bytecap_(x)
#define _In_opt_bytecap_(x)
#define _Inout_bytecap_(x)
#define _Out_cap_(x)
#define _Out_opt_cap_(x)
#define _In_cap_(x)
#define _In_opt_cap_(x)
#define _Inout_cap_(x)
#define _Out_z_bytecap_(x)
#define _Inout_z_bytecap_(x)
#define _Out_writes_all_(x)
#define _Out_writes_all_opt_(x)
#define _Outptr_result_bytebuffer_(x)
#define _Outptr_opt_result_bytebuffer_(x)
#define _Outptr_result_buffer_(x)
#define _Post_writable_byte_size_(x)
#define _Post_readable_byte_size_(x)
#define _Pre_writable_byte_size_(x)
#define _Pre_readable_byte_size_(x)

#endif
