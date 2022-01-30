#include <kissfft/kiss_fftr.h>
#include <math.h>

#define IABS(n)	sqrt (((n).i * (n).i) + ((n).r * (n).r))
#define F2I(f)	((int) (SRATE / (f)))

#define SAMPS	((size_t) (audlen / NFFT))

#define NFFT 1024
#define SRATE 44100
#define SENSE 1
kiss_fft_scalar *audbuf = NULL; size_t audlen = 0;


void fft (void) {
	float avglevel = 0;
	float *taud = new float [NFFT] [audlen]; // AMP = [FREQ] [POS]
	
	
	// STAGE 1:  FFT routine
	{	kiss_fftr_cfg kiss = kiss_fftr_alloc (NFFT, 0, NULL, NULL);
		
		float totalavg = 0;
		for (size_t audpos = 0; audpos != audlen; audpos ++) {
			float segavg = 0;
			
			// Process audio segment	
			kiss_fft_scalar out [NFFT];
			kiss_fftr (kiss, audbuf + (audpos * NFFT), &out);
			
			for (size_t i = 0; i != NFFT; i ++) {
				// Process item of OUT (individual cycle, frequencies)
				segavg += taud [i] [audpos] = IABS (out [i]);
			}
			
			totalavg += segavg / NFFT;
		}
		avglevel = totalavg / audlen;
		
		kiss_fftr_free (kiss);
	}
	
	
	// STAGE 2: take out noise
	{
		for (size_t audpos = 0; audpos != audlen; audpos ++) {
			for (size_t i = 0; i != NFFT; i ++) {
				if (taud [i] [audpos] > avglevel) {
					
				}
			}
		}
	}
	
	delete[] taud;
}

