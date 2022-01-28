#include <kiss_fftr.h>

#define NFFT 1023
kiss_fftr_cfg kiss = NULL;

void init (void) {
	kiss = kiss_fftr_alloc (NFFT + 1, 0, NULL, NULL);
}

void process (kiss_fft_scalar *audbuf) {
	kiss_fft_scalar freqs [NFFT];
	
	kiss_fftr (kiss, audbuf, &freqs);
	
	
