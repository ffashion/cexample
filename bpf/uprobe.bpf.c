// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include <linux/bpf.h>
#include <linux/ptrace.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("uprobe")
int BPF_KPROBE(uprobe_add, int a, int b)
{
	bpf_printk("uprobed_add ENTRY: a = %d, b = %d\n", a, b);
	return 0;
}

SEC("uretprobe")
int BPF_KRETPROBE(uretprobe_add, int ret)
{
	bpf_printk("uprobed_add EXIT: return = %d\n", ret);
	return 0;
}

SEC("uprobe//proc/self/exe:uprobed_sub")
int BPF_KPROBE(uprobe_sub, int a, int b)
{
	bpf_printk("uprobed_sub ENTRY: a = %d, b = %d\n", a, b);
	return 0;
}

SEC("uretprobe//proc/self/exe:uprobed_sub")
int BPF_KRETPROBE(uretprobe_sub, int ret)
{
	bpf_printk("uprobed_sub EXIT: return = %d\n", ret);
	return 0;
}

// SEC("uhello")
// int BPF_KPROBE(hello)
// {
// 	bpf_printk("hello Enter\n");
// 	return 0;
// }

