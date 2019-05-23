
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>		// sleep
#include <libaxidma.h>
#include <memory.h>
#include <stdint.h>

typedef struct {
    uint8_t red, green, blue;
} __attribute__ ((packed)) pixel_t;

static void fill_vertical_pattern(pixel_t* image, unsigned width, unsigned height);
static void fill_horizontal_pattern(pixel_t* image, unsigned width, unsigned height);
static void fill_diagonal_pattern(pixel_t* image, unsigned width, unsigned height);

int main()
{
    const size_t width = 1280;
    const size_t height = 960;
    const size_t bpp = sizeof(pixel_t);

	axidma_dev_t axidma_dev = axidma_init();
    if (axidma_dev == NULL) {
        fprintf(stderr, "Error: Failed to initialize the AXI DMA device.\n");
        return 1;
    }

    const array_t* array = axidma_get_vdma_tx(axidma_dev);
    if (array->len == 0) {
        printf("No vdma-tx channels found!\n");
        goto exit;
    }

    size_t frame_size = width * height * bpp;
    void* frame_buf = axidma_malloc(axidma_dev, frame_size);
    if (frame_buf == NULL) {
    	printf("axidma_malloc(%d) failed!\n", frame_size);
        goto exit;
    }

    fill_vertical_pattern(frame_buf, width, height);

    int channel = array->data[0];

    printf("Using channel id=%d...\n", channel);

    void* frames[] = { frame_buf };
    size_t frame_count = sizeof(frames)/sizeof(frames[0]);

    int result = axidma_video_transfer(axidma_dev, channel, width, height, bpp, frames, frame_count);
    if (result != 0) {
    	printf("axidma_video_transfer(%d) failed with %d!\n", channel, result);
    	goto free_and_exit;
    }

    printf("Press Enter to switch.\n");
    getchar();

    fill_horizontal_pattern(frame_buf, width, height);

    printf("Press Enter to switch.\n");
    getchar();

    fill_diagonal_pattern(frame_buf, width, height);

    printf("Press Enter to stop.\n");
    getchar();

    axidma_stop_transfer(axidma_dev, channel);

free_and_exit:
	axidma_free (axidma_dev, frame_buf, frame_size);
exit:
    axidma_destroy(axidma_dev);

	return 0;
}

static void fill_vertical_pattern(pixel_t* image, unsigned width, unsigned height)
{
    pixel_t* px = image;
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            px[j].red = j;
            px[j].green = j;
            px[j].blue = j;
        }
        px += width;
    }
}

static void fill_horizontal_pattern(pixel_t* image, unsigned width, unsigned height)
{
    pixel_t* px = image;
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            px[j].red = i;
            px[j].green = i;
            px[j].blue = i;
        }
        px += width;
    }
}

static void fill_diagonal_pattern(pixel_t* image, unsigned width, unsigned height)
{
    pixel_t* px = image;
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            px[j].red = i + j;
            px[j].green = i + j;
            px[j].blue = i + j;
        }
        px += width;
    }
}
