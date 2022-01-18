/* Factored discrete Fourier transform, or FFT, and its inverse iFFT */
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <fft.h>
#include <string.h>
#include "stm32f7xx_hal.h"
#include "fasttrig.h"
float fastsqrt(float val);
float sqrt1(float x)  ;
/* Print a vector of complexes as ordered pairs. */
static void print_vector(const char *title,
                         complex *x,
                         int n)
{
    int i;
    printf("%s (dim=%d):", title, n);
    for (i = 0; i < n; i++)
        printf(" %5.2f,%5.2f ", x[i].Re, x[i].Im);
    putchar('\n');
    return;
}

/*
   Алгоритм Кулі-Тьюкі
   https://bit.ly/3G6ckYm

   fft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute fft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = -sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void fft(complex *v, int n, complex *tmp)
{
    if (n > 1)
    {
        int k, m;
        complex z, w, *vo, *ve;
        ve = tmp;
        vo = tmp + n / 2;
        for (k = 0; k < n / 2; k++)
        {
            ve[k] = v[2 * k];
            vo[k] = v[2 * k + 1];
        }
        fft(ve, n / 2, v); /* FFT on even-indexed elements of v[] */
        fft(vo, n / 2, v); /* FFT on odd-indexed elements of v[] */
        for (m = 0; m < n / 2; m++)
        {
            w.Re = sin(2 * PI * m / (double)n);
            w.Im = -cos(2 * PI * m / (double)n);
            z.Re = w.Re * vo[m].Re - w.Im * vo[m].Im; /* Re(w*vo[m]) */
            z.Im = w.Re * vo[m].Im + w.Im * vo[m].Re; /* Im(w*vo[m]) */
            v[m].Re = ve[m].Re + z.Re;
            v[m].Im = ve[m].Im + z.Im;
            v[m + n / 2].Re = ve[m].Re - z.Re;
            v[m + n / 2].Im = ve[m].Im - z.Im;
        }
    }
    return;
}

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
void ifft(complex *v, int n, complex *tmp)
{
    if (n > 1)
    {
        int k, m;
        complex z, w, *vo, *ve;
        ve = tmp;
        vo = tmp + n / 2;
        for (k = 0; k < n / 2; k++)
        {
            ve[k] = v[2 * k];
            vo[k] = v[2 * k + 1];
        }
        ifft(ve, n / 2, v); /* FFT on even-indexed elements of v[] */
        ifft(vo, n / 2, v); /* FFT on odd-indexed elements of v[] */
        for (m = 0; m < n / 2; m++)
        {
            w.Re = cos(2 * PI * m / (double)n);
            w.Im = sin(2 * PI * m / (double)n);
            z.Re = w.Re * vo[m].Re - w.Im * vo[m].Im; /* Re(w*vo[m]) */
            z.Im = w.Re * vo[m].Im + w.Im * vo[m].Re; /* Im(w*vo[m]) */
            v[m].Re = ve[m].Re + z.Re;
            v[m].Im = ve[m].Im + z.Im;
            v[m + n / 2].Re = ve[m].Re - z.Re;
            v[m + n / 2].Im = ve[m].Im - z.Im;
        }
    }
    return;
}

/*
    fft2d(v, w, h)
    size of tmp should equal ((w>h)?w:h)*((w>h)?w:h)*sizeof(complex)
    size of tmp_fft should equal ((w>h)?w:h)*((w>h)?w:h)*sizeof(complex)
*/
void fft2d(complex *v, int w, int h, complex *tmp, complex *tmp_fft)
{
    // скопіюємо у тимчасовий буфер
    memcpy(tmp, v, w * h * sizeof(complex));
    print_vector("tmp after copy", tmp, w * h);
    // для кожного рядочку:
    for (int row = 0; row < h; row++)
    {
        fft(&tmp[row * w], w, &tmp_fft[row * w]);
    }
    print_vector("tmp after fft1", tmp, w * h);
    // рядки - у колонки
    transpose(tmp, tmp_fft, w, h);
    print_vector("tmp after fft1+transpose", tmp_fft, w * h);
    for (int row = 0; row < w; row++)
    {
        fft(&tmp_fft[row * h], h, tmp);
    }
    print_vector("tmp after fft1+transpose+fft2", tmp_fft, w * h);
    transpose(tmp_fft, v, w, h);
    print_vector("tmp after fft1+transpose+fft2+transpose", tmp_fft, w * h);
    // result in v
}
/*
   to_complex(__uint8_t *src, complex *dest, size_t sz)
   size of src - sz
   size of dest - sz * sizeof(complex)
*/
void to_complex(__uint8_t *src, complex *dest, size_t sz_src)
{
    for (int i = 0; i < sz_src; i++)
    {
        dest[i].Im = 0;
        dest[i].Re = src[i];
    }
}

/*
   from_complex(__uint8_t *src, complex *dest, size_t sz)
   size of dest - sz
   size of src - sz * sizeof(complex)
*/
void from_complex(complex*src, uint8_t*dest, size_t sz_dest)
{
    for (int i = 0; i < sz_dest; i++)
    {
        dest[i] = src[i].Re;
    }
}

/*
   abs_complex(__uint8_t *src, complex *dest, size_t sz)
   size of dest - sz
   size of src - sz * sizeof(complex)
*/
void abs_complex(complex*src, uint8_t*dest, size_t sz_dest)
{
    for (int i = 0; i < sz_dest; i++)
    {
        dest[i] = sqrt1(src[i].Re*src[i].Re+src[i].Im*src[i].Im);
    }
}
static inline float 
fastlog2 (float x)
{
    union { float f; uint32_t i; } vx = { x };
    union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    return y - 124.22551499f
             - 1.498030302f * mx.f 
             - 1.72587999f / (0.3520887068f + mx.f);
}

static inline float
fastlog (float x)
{
    return 0.69314718f * fastlog2 (x);
}

/*
   log_complex(__uint8_t *src, complex *dest, size_t sz)
   size of dest - sz
   size of src - sz * sizeof(complex)
*/
float sqrt1(float x)  
{
  union
  {
    int i;
    float x;
  } u;
  u.x = x;
  u.i = (1<<29) + (u.i >> 1) - (1<<22); 
  
  // Two Babylonian Steps (simplified from:)
  // u.x = 0.5f * (u.x + x/u.x);
  // u.x = 0.5f * (u.x + x/u.x);
  u.x =       u.x + x/u.x;
  u.x = 0.25f*u.x + x/u.x;

  return u.x;
}  
float fastsqrt(float val) {
static int tmp;
tmp = *(int *)&val;
tmp -= 1<<23;
tmp = tmp >> 1;
tmp += 1<<29;
return *(float *)&tmp;
}
void log_complex(complex*src, uint8_t*dest, size_t sz_dest)
{
    for (int i = 0; i < sz_dest; i++)
    {
        dest[i] = fastlog(sqrt1(src[i].Re*src[i].Re+src[i].Im*src[i].Im)+1000);
     //   printf("log_complex:%i %f\n", dest[i],log(sqrtf(src[i].Re*src[i].Re+src[i].Im*src[i].Im)));
    }
}

/*
    void RGB888_to_grey(__uint8_t*src, __uint8_t*dest, size_t greysz)
    size of src = size of grey * 3
    size of dst = size of grey
*/
void RGB888_to_grey(__uint8_t *src, __uint8_t *dest, size_t greysz)
{
    for (int i = 0; i < greysz; i++)
    {
        dest[i] = src[i * 3];
    }
}
/*
    void grey_to_RGB888(__uint8_t*src, __uint8_t*dest, size_t greysz)
    size of src = size of grey
    size of dst = size of grey * 3
*/
void grey_to_RGB888(__uint8_t *src, __uint8_t *dest, size_t greysz)
{
    for (int i = 0; i < greysz; i++)
    {
        int j = i * 3;
        dest[j] = src[i];
        dest[j + 1] = src[i];
        dest[j + 2] = src[i];
    }
}

// used matrix op-s
void transpose(complex *src, complex *dest, int w, int h)
{
    int i, j;
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {
            dest[j * h + i] = src[i * w + j];
        }
    }
}

__uint8_t *im_start(__uint8_t *pbmp)
{
    uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
    uint32_t address;
    uint32_t input_color_mode = 0;

    /* Get bitmap data address offset */
    index = *(uint16_t *)(pbmp + 10);
    index |= (*(uint16_t *)(pbmp + 12)) << 16;

    /* Read bitmap width */
    width = *(uint16_t *)(pbmp + 18);
    width |= (*(uint16_t *)(pbmp + 20)) << 16;

    /* Read bitmap height */
    height = *(uint16_t *)(pbmp + 22);
    height |= (*(uint16_t *)(pbmp + 24)) << 16;

    /* Read bit/pixel */
    bit_pixel = *(uint16_t *)(pbmp + 28);
    /* Get the layer pixel format */
#define CM_ARGB8888 ((uint32_t)0x00000000) /*!< ARGB8888 color mode */
#define CM_RGB888 ((uint32_t)0x00000001)   /*!< RGB888 color mode */
#define CM_RGB565 ((uint32_t)0x00000002)   /*!< RGB565 color mode */
#define CM_ARGB1555 ((uint32_t)0x00000003) /*!< ARGB1555 color mode */
#define CM_ARGB4444 ((uint32_t)0x00000004) /*!< ARGB4444 color mode */
#define CM_L8 ((uint32_t)0x00000005)       /*!< L8 color mode */
#define CM_AL44 ((uint32_t)0x00000006)     /*!< AL44 color mode */
#define CM_AL88 ((uint32_t)0x00000007)     /*!< AL88 color mode */
#define CM_L4 ((uint32_t)0x00000008)       /*!< L4 color mode */
#define CM_A8 ((uint32_t)0x00000009)       /*!< A8 color mode */

    if ((bit_pixel / 8) == 4)
    {
        input_color_mode = CM_ARGB8888;
    }
    else if ((bit_pixel / 8) == 2)
    {
        input_color_mode = CM_RGB565;
    }
    else
    {
        input_color_mode = CM_RGB888;
    }

    /* Bypass the bitmap header */
    pbmp += (index /*+ (width * (height - 1) * (bit_pixel / 8))*/);
    return pbmp;
}

__uint8_t *im_end(__uint8_t *pbmp)
{
    uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
    uint32_t address;
    uint32_t input_color_mode = 0;

    /* Get bitmap data address offset */
    index = *(uint16_t *)(pbmp + 10);
    index |= (*(uint16_t *)(pbmp + 12)) << 16;

    /* Read bitmap width */
    width = *(uint16_t *)(pbmp + 18);
    width |= (*(uint16_t *)(pbmp + 20)) << 16;

    /* Read bitmap height */
    height = *(uint16_t *)(pbmp + 22);
    height |= (*(uint16_t *)(pbmp + 24)) << 16;

    /* Read bit/pixel */
    bit_pixel = *(uint16_t *)(pbmp + 28);
    /* Get the layer pixel format */
#define CM_ARGB8888 ((uint32_t)0x00000000) /*!< ARGB8888 color mode */
#define CM_RGB888 ((uint32_t)0x00000001)   /*!< RGB888 color mode */
#define CM_RGB565 ((uint32_t)0x00000002)   /*!< RGB565 color mode */
#define CM_ARGB1555 ((uint32_t)0x00000003) /*!< ARGB1555 color mode */
#define CM_ARGB4444 ((uint32_t)0x00000004) /*!< ARGB4444 color mode */
#define CM_L8 ((uint32_t)0x00000005)       /*!< L8 color mode */
#define CM_AL44 ((uint32_t)0x00000006)     /*!< AL44 color mode */
#define CM_AL88 ((uint32_t)0x00000007)     /*!< AL88 color mode */
#define CM_L4 ((uint32_t)0x00000008)       /*!< L4 color mode */
#define CM_A8 ((uint32_t)0x00000009)       /*!< A8 color mode */

    if ((bit_pixel / 8) == 4)
    {
        input_color_mode = CM_ARGB8888;
    }
    else if ((bit_pixel / 8) == 2)
    {
        input_color_mode = CM_RGB565;
    }
    else
    {
        input_color_mode = CM_RGB888;
    }

    /* Bypass the bitmap header */
    pbmp += (index + (width * (height - 1) * (bit_pixel / 8)));
    return pbmp;
}

uint32_t im_width(__uint8_t *pbmp)
{
    uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
    uint32_t address;
    uint32_t input_color_mode = 0;

    /* Read bitmap width */
    width = *(uint16_t *)(pbmp + 18);
    width |= (*(uint16_t *)(pbmp + 20)) << 16;

    return width;
}

uint32_t im_height(__uint8_t *pbmp)
{
    uint32_t height = 0;

    /* Read bitmap height */
    height = *(uint16_t *)(pbmp + 22);
    height |= (*(uint16_t *)(pbmp + 24)) << 16;
    return height;
}

uint32_t im_bit_pixel(__uint8_t *pbmp)
{
    uint32_t bit_pixel = 0;

    /* Read bit/pixel */
    bit_pixel = *(uint16_t *)(pbmp + 28);
    return bit_pixel;
}



/*
 rotation
 */

/* Function to left Rotate arr[] of size n by 1*/
static void elleftRotatebyOne(uint8_t* arr, size_t sz);

/*Function to left rotate arr[] of size sz by d*/
void elleftRotate(uint8_t* arr, int d, size_t sz)
{
    int i;
    for (i = 0; i < d; i++)
        elleftRotatebyOne(arr, sz);
}

static void elleftRotatebyOne(uint8_t* arr, size_t sz)
{
    uint8_t temp = arr[0], i;
    for (i = 0; i < sz - 1; i++)
        arr[i] = arr[i + 1];
    arr[sz-1] = temp;
}

/* Function to right Rotate arr[] of size n by 1*/
static void elrightRotatebyOne(uint8_t* arr, size_t sz);

/*Function to left rotate arr[] of size sz by d*/
void elrightRotate(uint8_t* arr, int d, size_t sz)
{
    int i;
    for (i = 0; i < d; i++)
        elrightRotatebyOne(arr, sz);
}

static void elrightRotatebyOne(uint8_t* arr, size_t sz)
{
    uint8_t temp = arr[sz-1];
    for (int i = sz-1; i >0; i--) {
        arr[i] = arr[i-1];
    }
    arr[0] = temp;
}

/* Function to up Rotate col of MX[][] of width, height by 1*/
static void elupRotatebyOne(uint8_t* mx, size_t width, size_t height, int num);

/*Function to up rotate MX[][] of size width\height by d*/
void elupRotate(uint8_t* mx, size_t width, size_t height, int d, int num)
{
    int i;
    for (i = 0; i < d; i++)
        elupRotatebyOne(mx, width, height, num);
}

static void elupRotatebyOne(uint8_t* mx, size_t width, size_t height, int num)
{
    uint8_t temp = mx[num], i;
    for (i = num; i < width*(height-1); i+=width)
        mx[i] = mx[i + width];
    mx[num+width*(height-1)] = temp;
}

/* Function to down Rotate el of MX[][] of width, height by 1*/
static void eldownRotatebyOne(uint8_t* mx, size_t width, size_t height, int num);

/*Function to down rotate MX[][] of size width\height by d*/
void eldownRotate(uint8_t* mx, size_t width, size_t height, int d, int num)
{
    int i;
    for (i = 0; i < d; i++)
        eldownRotatebyOne(mx, width, height, num);
}

static void eldownRotatebyOne(uint8_t* mx, size_t width, size_t height, int num)
{
    uint8_t temp = mx[num+width*(height-1)];
    for (int i = num+width*(height-1); i > num+width; i-=width)
        mx[i] = mx[i-width];
    mx[num] = temp;
}

void fftshift(uint8_t*mx, size_t width, size_t height)
{
    size_t row_shift;
    if (height % 2 == 0)
        row_shift = height / 2;
     else
        row_shift = height / 2 + 1;
    size_t col_shift;
    if (width % 2 == 0)
        col_shift = width / 2;
    else
        col_shift = width / 2 + 1;
    // col shirt
    for(int i=0; i<height; i++) {
        elrightRotate(&mx[i*width], col_shift, width);
    }
    // row shift
    for(int i=0; i<width; i++) {
        eldownRotate(&mx[i*height], width, height, row_shift, i);
    }
}
