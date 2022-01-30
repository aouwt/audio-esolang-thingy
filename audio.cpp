#include <kissfft/kiss_fftr.h>
#include <math.h>

#define IABS(n)	sqrt (((n).i * (n).i) + ((n).r * (n).r))
#define F2I(f)	((kiss_fft_scalar) (SRATE / (f)))

#define SAMPS	((size_t) (audlen / NFFT))

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


void fft (void) {
	prglen = audlen / NFFT;
	
	
	float avglevel = 0;
	float taud [NFFT] [prglen]; // AMP = [FREQ] [POS]
	
	
	// STAGE 1:  FFT routine
	{	kiss_fftr_cfg kiss = kiss_fftr_alloc (NFFT, 0, NULL, NULL);
		
		double totalavg = 0;
		for (size_t audpos = 0; audpos != prglen; audpos ++) {
			float segavg = 0;
			
			// Process audio segment	
			kiss_fft_cpx out [NFFT];
			kiss_fftr (kiss, audbuf + (audpos * NFFT), out);
			
			for (size_t i = 0; i != NFFT; i ++) {
				// Process item of OUT (individual cycle, frequencies)
				segavg += taud [i] [audpos] = IABS (out [i]);
			}
			
			totalavg += segavg / NFFT;
		}
		avglevel = totalavg / prglen;
		
		kiss_fftr_free (kiss);
	}
	
	
	prg = new Program [prglen];
	// STAGE 2: take out noise
	{
		for (size_t prgpos = 0; prgpos != prglen; prgpos ++) {
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

