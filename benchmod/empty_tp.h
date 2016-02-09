#undef TRACE_SYSTEM
#define TRACE_SYSTEM empty_tp

#if !defined(_TRACE_EMPTY_MODULE_H) || defined(TRACE_HEADER_MULTI_READ)
#define  _TRACE_EMPTY_MODULE_H

#include <linux/tracepoint.h>

TRACE_EVENT(
	empty_ioctl_1b,
	TP_PROTO(int payload),
	TP_ARGS(payload),
	TP_STRUCT__entry(
		__field(int, payload)
	),
	TP_fast_assign(
		entry->payload = payload
	),
	TP_printk("payload = %d", __entry->payload)
);

#endif /* _TRACE_EMPTY_MODULE_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

/* this part must be outside protection */
#include <trace/define_trace.h>
