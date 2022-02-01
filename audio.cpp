#include <kissfft/kiss_fftr.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define IABS(n)	sqrt ((n.i * n.i) + (n.r * n.r))
#define F2I(f)	((kiss_fft_scalar) (SRATE / (f)))

#define SAMPS	((size_t) (audlen / NFFT))
#define EFFECTIVE_NFFT (NFFT * 2)

#define NFFT 1024
#define SRATE 44100
#define SENSE 1

typedef struct {
	kiss_fft_scalar *freq [NFFT] = { NULL };
	kiss_fft_scalar dat [NFFT] = { 0 };
	size_t datlen = 0;
} Program;

Program *prg = NULL; size_t prglen = 0;

kiss_fft_scalar *audbuf = NULL; size_t audlen = 0;



size_t get_fd_len (int fd) {
	size_t b;
	if (ioctl(fd, FIONREAD, &b)) {
		perror ("ioctl");
		_exit (1);
	}
	return b;
}

void fft (void) {
	prglen = audlen / EFFECTIVE_NFFT;
	
	
	float avglevel = 0;
	float taud [NFFT + 1] [prglen + 1]; // AMP = [FREQ] [POS]
	
	
	// STAGE 1:  FFT routine
	{	kiss_fftr_cfg kiss = kiss_fftr_alloc (EFFECTIVE_NFFT, 0, NULL, NULL);
		
		double totalavg = 0;
		for (size_t audpos = 0; audpos != prglen; audpos ++) {
			float segavg = 0;
			
			// Process audio segment	
			kiss_fft_cpx out [NFFT + 1];
			kiss_fftr (kiss, audbuf + (audpos * EFFECTIVE_NFFT), out);
			
			for (size_t i = 0; i != NFFT; i ++) {
				// Process item of OUT (individual cycle, frequencies)
				segavg += (taud [i] [audpos] = IABS (out [i]));
			}
			
			totalavg += segavg / NFFT;
		}
		avglevel = totalavg / prglen;
		
		kiss_fftr_free (kiss);
	}
	
	
	prg = new Program [prglen];
	// STAGE 2: take out noise
	{	for (size_t prgpos = 0; prgpos != prglen; prgpos ++) {
			for (size_t i = 0; i != NFFT; i ++) {
				if (taud [i] [prgpos] > avglevel) {
					prg [prgpos].dat [prg [prgpos].datlen] = i;
					prg [prgpos].freq [i] = &prg [prgpos].dat [prg [prgpos].datlen];
					prg [prgpos].datlen ++;
				}
			}
		}
	}
}



void load_audfile (const char *path) {
	int fds [2];
	
	fprintf (stderr, "Launching ffmpeg to load file...");
	
	if (pipe (fds) == -1) {
		perror ("pipe");
		_exit (1);
	}
	
	pid_t pid = fork ();
	if (pid == 0) { // Is child
	
		//stole from http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html ...
		while ((dup2 (fds [1], STDOUT_FILENO) == -1) && (errno == EINTR));
		close (fds [0]);
		close (fds [1]);
	
		execlp ("ffmpeg", "", "-loglevel", "0", "-i", path, "-acodec", "pcm_u8", "-ar", "48000", "-ac", "1", "-f", "u8", "-", NULL);
		
		perror ("exec");
		_exit (1);


	} else if (pid == -1) {
		perror ("fork");
		_exit (1);
	}
	
	close (fds [1]);
	waitpid (pid, NULL, 0);
	
	fprintf (stderr, "Done\n");
	fprintf (stderr, "Loading file...");
	// Load into audbuf
	{	int fd = fds [0];
		audlen = get_fd_len (fd);
		
		audbuf = (kiss_fft_scalar *) malloc (sizeof (kiss_fft_scalar) * (audlen + 1));
		if (audbuf == NULL) {
			perror ("malloc");
			_exit (1);
		}
		
		// cant use one big read () bc we need to cast!!
		for (size_t i = 0; i != audlen; i ++) {
			signed char b;
			read (fd, &b, 1);
			
			audbuf [i] = (kiss_fft_scalar) b;
		}
		
		close (fd);
	}
	fprintf (stderr, "Done\n");
	fprintf (stderr, "%i bytes loaded.\n", audlen);
}

int main (void) {
	load_audfile ("a.wav");
	fft ();
}
