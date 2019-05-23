
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <stdint.h>
#include <endian.h>
#include <libaxidma.h>

#include <inttypes.h>

const size_t width = 1600;
const size_t height = 1204;
const size_t bpp = sizeof(uint8_t);

static axidma_dev_t axidma_dev;
static void* frame_buf = NULL;
static int channel = -1;

static void callback(int channel_id, void *data)
{
    unsigned line_len = width * bpp;
    unsigned tail_lines = 2;
    uint8_t* pos = data;
    pos += (height - tail_lines) * line_len;
    uint64_t blockid = *(uint64_t*)pos;

    // start new rx-transfer

    void* frames[] = { frame_buf };
    size_t frame_count = sizeof(frames)/sizeof(frames[0]);
    int result = axidma_video_transfer(axidma_dev, channel, width, height, bpp, frames, frame_count);
    if (result != 0) {
    	printf("%s: axidma_video_transfer(%d) failed with %d!\n", __func__, channel, result);
    }

    printf("%s:%d: channel_id=%d, data=%p, blockid=%llu\n\n", __FILE__, __LINE__, channel_id, data, blockid);
}

int main()
{
	axidma_dev = axidma_init();
    if (axidma_dev == NULL) {
        fprintf(stderr, "Error: Failed to initialize the AXI DMA device.\n");
        return 1;
    }

    const array_t* array = axidma_get_vdma_rx(axidma_dev);
    if (array->len == 0) {
        printf("No vdma-rx channels found!\n");
        goto exit;
    }

    size_t frame_size = width * height * bpp;
    frame_buf = axidma_malloc(axidma_dev, frame_size);
    if (frame_buf == NULL) {
    	printf("axidma_malloc(%d) failed!\n", frame_size);
        goto exit;
    }

    memset(frame_buf, 0, frame_size);

    channel = array->data[0];

    axidma_set_callback(axidma_dev, channel, callback, frame_buf);

    void* frames[] = { frame_buf };
    size_t frame_count = sizeof(frames)/sizeof(frames[0]);

    int result = axidma_video_transfer(axidma_dev, channel, width, height, bpp, frames, frame_count);
    if (result != 0) {
    	printf("axidma_video_transfer(%d) failed with %d!\n", channel, result);
    	goto free_and_exit;
    }

    printf("Press Enter to stop.\n");
    getchar();

    axidma_stop_transfer(axidma_dev, channel);

free_and_exit:
	axidma_free(axidma_dev, frame_buf, frame_size);
exit:
    axidma_destroy(axidma_dev);

	return 0;
}

