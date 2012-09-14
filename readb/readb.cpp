// (C) John Peterson. License GNU GPL 3.
// Binary stdin/stdout pass-through. The program ends when stdin is not received for t seconds. The timeout counter begin after the first input is received. The program can parse FLV input to provide continuous FLV output by suppressing the FLV header and creating a continuous timestamp.
#include "../common/common.h"
#define SIZE 0x2000
string sig_s = "";
struct sigaction sa;
bool nohead = false;
u32 to = 5;
string tmpf;
string opts = "HVv:ht:s:";
int opt[] = {'H', 'V', 'v', 'h', 't', 's'};
string optl[] = {"help", "version", "verbosity", "skip-header", "timeout", "signal-process"};
void usage() {
	fprintf(stdout, "Usage: readb [-H] [-v] [-t seconds] [-h] [-s process]\n"
		"\t-%c, --%s\tdisplay this message.\n"
		"\t-%c, --%s\tdisplay version.\n"
		"\t-%c, --%s\tlog verbosity.\n"
		"\t-%c, --%s\tstdin read timeout s. Ex: '%u' (default).\n"
		"\t-%c, --%s\tskip media header.\n"
		"\t-%c, --%s\tprocess name to send signal 9 to on stdin timeout.\n",
		opt[0], optl[0].c_str(),
		opt[1], optl[1].c_str(),
		opt[2], optl[2].c_str(),
		opt[3], optl[3].c_str(), to,
		opt[4], optl[4].c_str(),
		opt[5], optl[5].c_str());
}
bool get_arg(int argc, char** argv) {
	u32 i = 0;
	struct option longopts[] = {
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{NULL,				0,					NULL,	0}
	};
	int c;	
	while ((c = getopt_long(argc, argv, opts.c_str(), longopts, 0)) != -1) {
		switch (c) {
		case 'H':
			usage();
			return true;
		case 'V':
			log("%s %s\n", VERDATE, VER);
			return true;
		case 'v':
			verb = atoi(optarg);
			break;
		case 's':
			sig_s = string(optarg);
			break;
		case 'h':
			nohead = true;
			break;
		case 't':
			to = atoi(optarg);
			break;
		default:
			fprintf(stderr, "unknown option: %c\n", c);
			break;
		}
	}
	return false;
}
void sighandler(int sig, siginfo_t *info, void *ptr) {
	unsigned long pid = (unsigned long)info->si_pid;
	log("Received signal %d from %lu '%s'\n", sig, pid, pid2cmd(pid).c_str());
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
}
// parse flv
void read_flv(u8* buf, u32& size) {
	u8 buf_t[SIZE*4], buf_h[8];
	static u8 buf_r[SIZE];
	u32 len_t = 0;
	static bool first = true;
	static u8 size_r = 0;
	static u32 p = 0, time = 0, time_s = 0;
	static u64 written = 0;
	static int i = 0;
	log2("\n");
	if (first) {
		p = 0xd;
		if (nohead) {
			if (!read_line(1, tmpf).empty())
				istringstream(read_line(1, tmpf))>>hex >> len_t;
			if (!read_line(2, tmpf).empty())
				istringstream(read_line(2, tmpf))>>hex >> time_s;
			log2("Saved position %08x\n", len_t);
			log2("Saved time %08x\n", time_s);
		} else {
			memcpy(buf_t, buf, 0xd);
			len_t = 0xd;
		}
		if (!read_line(3, tmpf).empty())
		istringstream(read_line(3, tmpf))>>hex >> written;
		written += len_t;
	} else {
		if (p) {
			// log2("Partial write start size:%08x of %08x\n", p, size);
			u32 size_p = p > size ? size : p;
			memcpy(buf_t+len_t, buf, size_p);
			len_t += size_p;
			written += size_p;
			if (p >= size) {
				goto save;
			}
		}
	}
	log2("Start position %08x -%d of %08x %d\n", p, size_r, size, i);
	while (p+8 <= size) {
		// read header
		if (size_r) {
			// log2("Partial read rest size:%08x of %08x\n", size_r, size);	
			memcpy(buf_h, buf_r, size_r);
			memcpy(buf_h+size_r, buf, 8-size_r);
		} else
			memcpy(buf_h, buf+p, 8);
		u8 type = buf_h[0];
		if (type != 0x8 && type != 0x9 && type != 0x12) {
			fprintf(stderr, "Faulty type at %08x of %08x\n", p, size);
			exit(1);
		}
		u32 len;
		memcpy_rev(&len, buf_h+1, 3);
		memcpy_rev(((u8*)&time)+0, buf_h+4, 3); memcpy(((u8*)&time)+3, buf_h+7, 1);
		time += time_s;
		// write header
		memcpy(buf_t+len_t, &type, 1);
		memcpy_rev(buf_t+len_t+1, &len, 3);
		memcpy_rev(buf_t+len_t+4, ((u8*)&time)+0, 3); memcpy(buf_t+len_t+7, ((u8*)&time)+3, 1);
		u32 len_tag = len+7;
		log2("type:%02x size:%06x pos:%08x time:%08x npos:%08x tpos:%08llx\n", type, len, p, time, p+len_tag, written);
		if (type != 0x12 || !nohead) {
			len_t += 8;
			written += 8;
		}
		p += 8-size_r;
		size_r = 0;
		if (type != 0x12 || !nohead) {
			if (p+len_tag <= size) {
				// log2("Full write size:%06x frompos:%08x topos:%08x\n", len_tag, p, len_t);
				memcpy(buf_t+len_t, buf+p, len_tag);
				len_t += len_tag;
				written += len_tag;
			} else {
				// log2("Partial write size:%06x frompos:%08x topos:%08x\n", size-p, p, len_t);
				memcpy(buf_t+len_t, buf+p, size-p);
				len_t += size-p;
				written += size-p;
			}
		}
		p += len_tag;
	}
	if (p < size) {
		// log2("rest copy size:%06x pos:%08x\n", size-p, p);
		memcpy(buf_r, buf+p, size-p);
		size_r = size-p;
		p = size;
	}
save:
	p -= size;
	write_file(p, tmpf);
	write_file(time,tmpf,true);
	write_file(written,tmpf,true);
	size = len_t;
	memcpy(buf, buf_t, len_t);
	first = false;
	i++;
}
int main(int argc, char **argv) {
	u8 buf[SIZE*4];
	const u32 len = SIZE;
	u32 size;
	u64 written = 0;
	freopen(NULL, "rb", stdin);
	freopen(NULL, "wb", stdout);
	tmpf = home_dir() + "/.readb";
	if (get_arg(argc, argv)) return 0;
	
	// register signal handler
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = sighandler;
	sa.sa_flags = SA_SIGINFO;
	for (int i = 1; i <= 30; i++)
		sigaction(i, &sa, NULL);
	
	while (true) {
		size = read(STDIN_FILENO, buf, len);
		if (size == 0) return 0;
		read_flv(buf, size);
		write(STDOUT_FILENO, buf, size);
		written += size;
		if (!is_stdin(to)) break;
	}
	
	fprintf(stderr, "\nstdin timeout\n");
	if (!sig_s.empty()) send_signal(pn2pid(sig_s));
	return 1;
}