#ifndef fft_h
#define fft_h
#include<stdint.h>
typedef float real;
typedef struct
{
    real Re;
    real Im;
} complex;

#ifndef PI
#define PI 3.14159265358979323846
#endif

/*
 Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = -sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void fft(complex *v, int n, complex *tmp);


/* 
   ifft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute ifft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute ifft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void ifft(complex *v, int n, complex *tmp);

void fft2d(complex *v, int w, int h, complex *tmp, complex*tmp_fft);

void to_complex(__uint8_t *src, complex *dest, size_t sz_src);

/*
   from_complex(__uint8_t *src, complex *dest, size_t sz)
   size of dest - sz
   size of src - sz * sizeof(complex)
*/
void from_complex(complex*src, uint8_t*dest, size_t sz_dest);

/*
   abs_complex(__uint8_t *src, complex *dest, size_t sz)
   size of dest - sz
   size of src - sz * sizeof(complex)
*/
void abs_complex(complex*src, uint8_t*dest, size_t sz_dest);

/*
   log_complex(__uint8_t *src, complex *dest, size_t sz)
   size of dest - sz
   size of src - sz * sizeof(complex)
*/
void log_complex(complex*src, uint8_t*dest, size_t sz_dest);
/*
    void RGB888_to_grey(__uint8_t*src, __uint8_t*dest, size_t greysz)   
    size of src = size of grey * 3
    size of dst = size of grey
*/
void RGB888_to_grey(__uint8_t*src, __uint8_t*dest, size_t greysz);
/*
    void grey_to_RGB888(__uint8_t*src, __uint8_t*dest, size_t greysz)
    size of src = size of grey
    size of dst = size of grey * 3
*/
void grey_to_RGB888(__uint8_t*src, __uint8_t*dest, size_t greysz);

__uint8_t *im_start(__uint8_t *pbmp);
__uint8_t *im_end(__uint8_t *pbmp);
uint32_t im_width(__uint8_t *pbmp);
uint32_t im_height(__uint8_t *pbmp);
uint32_t im_bit_pixel(__uint8_t *pbmp);

// used matrix op-s
void transpose(complex *src, complex *dest, int w, int h);
/*Function to left rotate arr[] of size sz by d*/
void elrightRotate(uint8_t* arr, int d, size_t sz);
/*Function to left rotate arr[] of size sz by d*/
void elleftRotate(uint8_t* arr, int d, size_t sz);
/*Function to down rotate MX[][] of size width\height by d*/
void eldownRotate(uint8_t* mx, size_t width, size_t height, int d, int num);
/*Function to up rotate MX[][] of size width\height by d*/
void elupRotate(uint8_t* mx, size_t width, size_t height, int d, int num);

void fftshift(uint8_t*mx, size_t width, size_t height);

#endif //fft_h