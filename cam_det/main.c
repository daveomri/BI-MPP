#include <stdio.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>



int main(int32_t sR, int32_t sG, int32_t sB, int32_t  tlrc){
    // Opening the device
    int fd = open("/dev/video3", O_RDWR);

    if (fd < 0){
        printf("Unsuccessful opening of the device\n");
        return 1;
    }

    // Is device available for capturing frames
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        printf("Something went wrong.\n");
        return 1;
    } 

    // setting image format
    struct v4l2_format img_form;
    img_form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    img_form.fmt.pix.width = 720;
    img_form.fmt.pix.height = 1280;
    img_form.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;//V4L2_PIX_FMT_MJPEG;
    img_form.fmt.pix.field = V4L2_FIELD_NONE;
    // tell the device you are using this format
    if(ioctl(fd, VIDIOC_S_FMT, &img_form) < 0){
        printf("Error: format not set");
        return 1;
    }

    // Buffers from the device
    struct v4l2_requestbuffers req_buf = {0};
    req_buf.count = 1; // one request buffer
    req_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we an use for capturing frames
    req_buf.memory = V4L2_MEMORY_MMAP;
    if(ioctl(fd, VIDIOC_REQBUFS, &req_buf) < 0){
        printf("Error: buffers not asign");
        return 1;
    }

    // Buffer informations
    struct v4l2_buffer query_buf = {0};
    query_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    query_buf.memory = V4L2_MEMORY_MMAP;
    query_buf.index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, &query_buf) < 0){
            printf("Error: no info given");
            return 1;
    }


    char* buffer = (char*)mmap(NULL, query_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd, query_buf.m.offset);
    memset(buffer, 0, query_buf.length);


    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        printf("Error: stream couldn't be established");
        return 1;
    }



    int32_t R = 0;
    int32_t G = 0;
    int32_t B = 0;


    //loop goes here
    

    //printf("Buffer has %d KB\n", bufferinfo.bytesused/1024);
    int i = 0;
    int n_points = 0;
    int n_lines = 0;

    int r_i = 1;
    int c_points = 0;
    int oif=0;
    while (1){
        if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
            perror("Could not queue buffer, VIDIOC_QBUF");
            return 1;
        }

        if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
            perror("Could not dequeue the buffer, VIDIOC_DQBUF");
            return 1;
        }

        if (r_i==1){
            printf("/\n");
            r_i--;
        }
        else{
            printf("\\\n");
            r_i++;
        }

        
        i = 0;
        n_points = 0;
        n_lines = 0;

        c_points = 0;
        for (i = 0; i<(bufferinfo.bytesused); i++){
            if (c_points==1280){
                n_lines++;
                c_points = 0;
                n_points = 0;
            }
            
            B = ((((int32_t)buffer[i])>>16)&0x0000ff);
            G = ((((int32_t)buffer[i])>>8)&0x0000ff);
            R = (((int32_t)buffer[i])&0x000000ff);

            if( ((sR-R)*(sR-R)+(sG-G)*(sG-G)+(sB-B)*(sB-B)) < tlrc  )
                n_points++;
            c_points++;

            // printf("");
            // printf("R %u\n", R);
            // printf("G %u\n", G);
            // printf("B %u\n", B);
            // printf("------------------------\n");
        }
        if (n_lines > 15){
            //printf("Matched points: %d\n", n_points);
            printf("COLOR DETECTED\n");
            oif=1;
        }
        else if (oif==1){
            printf("COLOR LOST\n");
            oif=0;
        }
        sleep(1);
    }
    

    
    //end of loop

    /**
     * Zastaveni stremovani kamery
     */
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        printf("Error: stream couldn't be stopped");
        return 1;
    }

    // Closing the device
    close(fd);
    return 0;
}