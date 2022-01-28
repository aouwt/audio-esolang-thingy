#include <kiss_fftr.h>
#include <math.h>

#define IABS(n)	sqrt (((n).i * (n).i) + ((n).r * (n).r))
#define F2I(f)	((int) (SRATE / (f)))

#define SAMPS	((size_t) (audlen / NFFT))

#define NFFT 1024
#define SRATE 44100
#define SENSE 1
kiss_fft_scalar *audbuf = NULL;
int nfft;

typedef struct {
	float amp [NFFT];
} Processed;

// <AMPLITUDE> == Processed [<POSITION>].amp [F2I (<FREQUENCY>)]

void process (Processed aud []) {
	for (size_t audpos = 0; audpos != audlen; audpos ++) {
		// Process audio segment
		
		kiss_fft_scalar out [NFFT];
		kiss_ffr_cfg kiss = kiss_fftr_alloc (NFFT, 0, NULL, NULL);
		kiss_fftr (kiss, audbuf + (audpos * NFFT), &out);
		
		for (size_t i = 0; i != NFFT; i ++) {
			// Process item of OUT (individual cycle, frequencies)
			aud [audpos].amp [i] = IABS (out [i]);
		}
		
		kiss_fftr_free (kiss);
	}
}

void interp (Processed aud []) {
	float amp;
	
	#define DEF(freq, code) \
		if ((amp = aud [i].amp [F2I (freq)]) >= SENSE) amp;
	
	for (size_t i = 0; i != SAMPS; i ++) {
		DEF (440, {
			printf ("hi");
		})
	}
	#undef DEF
}
