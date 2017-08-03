/*==============================================================================
Copyright (c) 2017, Windhover Labs
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of CmdIn nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY VCPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, VCEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "vc_dev_io_v4l.h"
#include "cfe.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include "vc_app.h"
#include <linux/videodev2.h>


typedef enum
{
    VC_DEVICE_UNUSED       = 0,
    VC_DEVICE_DISABLED     = 1,
    VC_DEVICE_ENABLED      = 2
} VC_DeviceMode_t;


typedef enum
{
    VC_DEVICE_UNINITIALIZED = 0,
    VC_DEVICE_INITIALIZED   = 1,
    VC_DEVICE_STREAMING     = 2
} VC_DeviceStatus_t;


typedef struct
{
    VC_DeviceStatus_t   Status;
    VC_DeviceMode_t     Mode;
    uint8               ChannelID;
    char                DevName[VC_MAX_DEVICE_PATH];
    int                 DeviceFd;
    uint32              BufferType;
    uint32              FrameWidth;
    uint32              FrameHeight;
    uint32              VideoFormat;
    uint8               FieldOrder;
    uint8               BufferRequest;
    uint32              MemoryType;
    char                Buffers[VC_V4L_BUFFER_REQUEST][VC_MAX_BUFFER_SIZE];
    uint32              Buffer_Size;
} VC_Device_Handle_t;


typedef struct
{
    VC_Device_Handle_t  Channel[VC_MAX_DEVICES];
} VC_AppCustomDevice_t;


VC_AppCustomDevice_t VC_AppCustomDevice = {
    {
        { 
          .Status            = VC_DEVICE_UNINITIALIZED, \
          .Mode              = VC_DEVICE_ENABLED, \
          .ChannelID         = 0, \
          .DevName           = VC_DEVICE_PATH, \
          .DeviceFd          = 0, \
          .BufferType        = VC_V4L_BUFFER_TYPE, \
          .FrameWidth        = VC_FRAME_WIDTH, \
          .FrameHeight       = VC_FRAME_HEIGHT, \
          .VideoFormat       = VC_V4L_VIDEO_FORMAT, \
          .FieldOrder        = VC_V4L_VIDEO_FIELD_ORDER, \
          .BufferRequest     = VC_V4L_BUFFER_REQUEST, \
          .MemoryType        = VC_V4L_MEMORY_TYPE, \
          .Buffers           = {{0}}, \
          .Buffer_Size       = VC_MAX_BUFFER_SIZE
        }
    }
};


int32 VC_EnableDevice(uint8 DeviceID, const char *DeviceName);
int32 VC_DisableDevice(uint8 DeviceID);
int32 VC_ConfigureDevice(uint8 DeviceID);
int32 VC_Start_StreamingDevice(uint8 DeviceID);
int32 VC_Stop_StreamingDevice(uint8 DeviceID);
int32 VC_Start_Streaming(void);
int32 VC_Stop_Streaming(void);
int32 VC_Send_Buffer(uint8 DeviceID);


static int VC_Ioctl(int fh, int request, void *arg)
{
        int r;

        do 
        {
            r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

int32 VC_ConfigureDevice(uint8 DeviceID)
{
    int32 returnCode = 0;
    uint32 i = 0;
    struct v4l2_format              Format;
    struct v4l2_capability          Capabilities;
    struct v4l2_requestbuffers      Request;
    struct v4l2_buffer              Buffer;

    bzero(&Format, sizeof(Format));
    Format.type                = VC_AppCustomDevice.Channel[DeviceID].BufferType;
    Format.fmt.pix.width       = VC_AppCustomDevice.Channel[DeviceID].FrameWidth;
    Format.fmt.pix.height      = VC_AppCustomDevice.Channel[DeviceID].FrameHeight;
    Format.fmt.pix.pixelformat = VC_AppCustomDevice.Channel[DeviceID].VideoFormat;
    Format.fmt.pix.field       = VC_AppCustomDevice.Channel[DeviceID].FieldOrder;

    bzero(&Request, sizeof(Request));
    Request.count              = VC_AppCustomDevice.Channel[DeviceID].BufferRequest;
    Request.type               = VC_AppCustomDevice.Channel[DeviceID].BufferType;
    Request.memory             = VC_AppCustomDevice.Channel[DeviceID].MemoryType;

    if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_QUERYCAP, &Capabilities)) 
    {            
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC VIDIOC_QUERYCAP returned %i on %s channel %u", errno,
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
    if (!(Capabilities.capabilities & VC_AppCustomDevice.Channel[DeviceID].BufferType)) 
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC Capabilities %u on %s channel %u not found", 
                        (unsigned int)VC_AppCustomDevice.Channel[DeviceID].BufferType,
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
    if (!(Capabilities.capabilities & V4L2_CAP_STREAMING)) 
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC Capabilities %u on %s channel %u not found", 
                        V4L2_CAP_STREAMING,
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }

    VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_S_FMT, &Format);
    
    if (Format.fmt.pix.pixelformat != VC_AppCustomDevice.Channel[DeviceID].VideoFormat)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC device %s channel %u didn't accept format %u.",
                        VC_AppCustomDevice.Channel[DeviceID].DevName,
                        (unsigned int)DeviceID, 
                        (unsigned int)VC_AppCustomDevice.Channel[DeviceID].VideoFormat);
        returnCode = -1;
        goto end_of_function;
    }
    
    if ((Format.fmt.pix.width != VC_AppCustomDevice.Channel[DeviceID].FrameWidth) || 
    (Format.fmt.pix.height != VC_AppCustomDevice.Channel[DeviceID].FrameHeight))
    {
         CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC device %s channel %u didn't accept resolution instead %d:%d.",
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID, 
                        Format.fmt.pix.width, Format.fmt.pix.height);
        returnCode = -1;
        goto end_of_function;
    }
    
    if (Format.fmt.pix.sizeimage != VC_AppCustomDevice.Channel[DeviceID].Buffer_Size)
    {
         CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC device %s channel %u size image %u > buffer size %lu.",
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID, 
                        Format.fmt.pix.sizeimage, VC_AppCustomDevice.Channel[DeviceID].Buffer_Size);
        returnCode = -1;
        goto end_of_function;
    }
    
    if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_REQBUFS, &Request))
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC VIDIOC_REQBUFS returned %i on %s channel %u", errno,
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
    if (Request.count != VC_AppCustomDevice.Channel[DeviceID].BufferRequest)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC VIDIOC_REQBUFS did not comply. Only %u buffers on %s channel %u", Request.count, 
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
    for (i=0; i < VC_AppCustomDevice.Channel[DeviceID].BufferRequest; i++)
    {
        bzero(&Buffer, sizeof(Buffer));
        Buffer.type            = VC_AppCustomDevice.Channel[DeviceID].BufferType;
        Buffer.memory          = VC_AppCustomDevice.Channel[DeviceID].MemoryType;
        Buffer.index           = i;
        Buffer.m.userptr       = (unsigned long)&VC_AppCustomDevice.Channel[DeviceID].Buffers[i][0];
        Buffer.length          = VC_AppCustomDevice.Channel[DeviceID].Buffer_Size;

        if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_QBUF, &Buffer))
        {
            CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC VIDIOC_QBUF returned %i on %s channel %u", errno,
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
            returnCode = -1;
            goto end_of_function;
        }
    }
    
end_of_function:
    return returnCode;
}


int32 VC_Start_StreamingDevice(uint8 DeviceID)
{
    int32 returnCode = 0;
    enum v4l2_buf_type              Type;
    
    Type = VC_AppCustomDevice.Channel[DeviceID].BufferType;

    if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_STREAMON, &Type))
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC VIDIOC_STREAMON returned %i on %s channel %u", errno,
                    VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
    }
    else
    {
        VC_AppCustomDevice.Channel[DeviceID].Status = VC_DEVICE_STREAMING;
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC VIDIOC_STREAMON success on %s channel %u", 
                    VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
    }

    return returnCode;
}


int32 VC_Stop_StreamingDevice(uint8 DeviceID)
{
    int32 returnCode = 0;
    uint32 i = 0;
    enum v4l2_buf_type Type;
    
    Type = VC_AppCustomDevice.Channel[DeviceID].BufferType;
    
    if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_STREAMOFF, &Type))
    {
            CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC VIDIOC_STREAMOFF returned %i on %s channel %u", errno,
                        VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
            returnCode = -1;
    }
    else
    {
        VC_AppCustomDevice.Channel[DeviceID].Status = VC_DEVICE_INITIALIZED;
            CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC VIDIOC_STREAMOFF success on %s channel %u", 
                    VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
    }
    return returnCode;
}


int32 VC_Send_Buffer(uint8 DeviceID)
{
    int32 returnCode = 0;
    uint32 i = 0;
    
    struct v4l2_buffer Buffer;
    
    bzero(&Buffer, sizeof(Buffer));
    Buffer.type            = VC_AppCustomDevice.Channel[DeviceID].BufferType;
    Buffer.memory          = VC_AppCustomDevice.Channel[DeviceID].MemoryType;

    if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_DQBUF, &Buffer))
    {
        if(EAGAIN == errno)
        {
            goto end_of_function;
        }
        else
        {
            CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC VIDIOC_DQBUF returned %i on %s channel %u", errno,
                    VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
            returnCode = -1;
            goto end_of_function;
        }
    }
    
    if (VC_MAX_PACKET_SIZE < Buffer.bytesused)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                "VC Packet on %s channel %u is too large",
                VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
    }
    
    /* Send data, for now map device id to senddata channel */
    if (-1 == VC_SendData(DeviceID, (void*)Buffer.m.userptr, Buffer.bytesused))
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                "VC send data failed on %s channel %u",
                VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
    if (-1 == VC_Ioctl(VC_AppCustomDevice.Channel[DeviceID].DeviceFd, VIDIOC_QBUF, &Buffer))
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                "VC VIDIOC_QBUF returned %i on %s channel %u", errno,
                VC_AppCustomDevice.Channel[DeviceID].DevName, (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
end_of_function:
    return returnCode;
}


int32 VC_Start_Streaming(void)
{
    int32 returnCode = 0;
    
    uint32 i = 0;
    uint32 maxFd = 0;
    fd_set fds;
    
    struct timeval timeValue;
    timeValue.tv_sec = VC_BUFFER_FILL_TIMEOUT_SEC;
    timeValue.tv_usec = VC_BUFFER_FILL_TIMEOUT_USEC;
    
    do
    {
        maxFd = 0;
        returnCode = 0;

        /* initialize the set */
        FD_ZERO(&fds);
        
        /* add enabled devices to the set */
        for (i=0; i < VC_MAX_DEVICES; i++)
        {
            if(VC_AppCustomDevice.Channel[i].Mode == VC_DEVICE_ENABLED 
            && VC_AppCustomDevice.Channel[i].Status == VC_DEVICE_STREAMING)
            {
                FD_SET(VC_AppCustomDevice.Channel[i].DeviceFd, &fds);
            
                /* Get the greatest fd value for select() */
                if (VC_AppCustomDevice.Channel[i].DeviceFd > maxFd)
                {
                    maxFd = VC_AppCustomDevice.Channel[i].DeviceFd; 
                }
            }
        }
    
        if (maxFd > 0)
        {
            returnCode = select(maxFd + 1, &fds, 0, 0, &timeValue);
        }
        else
        {
            CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC can't start streaming no devices are streaming enabled");
            returnCode = -1;
            goto end_of_function;
        }
    
        if (-1 == returnCode)
        {
            /* select was interrupted, try again */
            if (EINTR == errno)
            {
                CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC select was interrupted");
                continue;
            }
            else
            {
                CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC start streaming failed select() returned %i", errno);
                returnCode = -1;
                goto end_of_function;
            }
        }
    
        /* select timed out */
        if (0 == returnCode)
        {
            CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "VC buffer timeout streaming stopped");
            returnCode = -1;
            goto end_of_function;
        } 
    
        if(returnCode > 0)
        {    for (i=0; i < VC_MAX_DEVICES; i++)
            {
                if(VC_AppCustomDevice.Channel[i].Mode == VC_DEVICE_ENABLED 
                && VC_AppCustomDevice.Channel[i].Status == VC_DEVICE_STREAMING)
                {
                    if(FD_ISSET(VC_AppCustomDevice.Channel[i].DeviceFd, &fds))
                    {
                        VC_Send_Buffer(i);
                    }
                }
            }
        }    
    } while (maxFd > 0);

end_of_function:
    return returnCode;
}


int32 VC_Stop_Streaming(void)
{
    uint32 i = 0;
    int32 returnCode = 0;
    
    for(i=0; i < VC_MAX_DEVICES; i++)
    {
        if(VC_AppCustomDevice.Channel[i].Mode == VC_DEVICE_ENABLED)
        {
            if(VC_AppCustomDevice.Channel[i].Status == VC_DEVICE_STREAMING)
            {    
                if(-1 == VC_Stop_StreamingDevice(i))
                {
                    returnCode = -1;
                }
            }
        }
    }
    return returnCode;
}

int32 VC_EnableDevice(uint8 DeviceID, const char *DeviceName)
{
    int32 returnCode = 0;
    uint32 i = 0;

    if((VC_AppCustomDevice.Channel[DeviceID].Mode == VC_DEVICE_ENABLED) &
        (VC_AppCustomDevice.Channel[DeviceID].DeviceFd != 0))
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC Device %s for channel %u already enabled.", DeviceName, (unsigned int)i);
        returnCode = -1;
        goto end_of_function;
    }

    if(DeviceName == 0)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC Device name for channel %u is null.", (unsigned int)i);
        returnCode = -1;
        goto end_of_function;
    }

    if(DeviceID >= VC_MAX_DEVICES)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC DeviceID (%u) invalid.", (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }
    
    if((VC_AppCustomDevice.Channel[DeviceID].DeviceFd = open(DeviceName, O_RDWR | O_NONBLOCK, 0)) < 0)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "VC Device open errno: %i on channel %u", errno, (unsigned int)i);
        VC_AppCustomDevice.Channel[DeviceID].Mode = VC_DEVICE_DISABLED;
        returnCode = -1;
        goto end_of_function;
    }

end_of_function:
    return returnCode;
}


int32 VC_InitDevices(void)
{
    uint32 i = 0;
    int32 returnCode = 0;

    for (i=0; i < VC_MAX_DEVICES; i++)
    {
        if(VC_AppCustomDevice.Channel[i].Mode == VC_DEVICE_ENABLED)
        {
            if (returnCode = VC_EnableDevice(i, VC_AppCustomDevice.Channel[i].DevName))
            {
                VC_AppCustomDevice.Channel[i].Mode = VC_DEVICE_DISABLED;
            }
            else
            {
                CFE_EVS_SendEvent(VC_DEV_INF_EID, CFE_EVS_INFORMATION,
                        "VC Device enabled channel %u from %s",
                        (unsigned int)i, VC_AppCustomDevice.Channel[i].DevName);
            }
            
            if (returnCode = VC_ConfigureDevice(i))
            {
                VC_AppCustomDevice.Channel[i].Status = VC_DEVICE_UNINITIALIZED;
            }
            else
            {
                VC_AppCustomDevice.Channel[i].Status = VC_DEVICE_INITIALIZED;
                CFE_EVS_SendEvent(VC_DEV_INF_EID, CFE_EVS_INFORMATION,
                        "VC Device configured channel %u from %s",
                        (unsigned int)i, VC_AppCustomDevice.Channel[i].DevName);
            }
            
            if (returnCode = VC_Start_StreamingDevice(i))
            {
                /* do nothing */
            }
            else
            {
                VC_AppCustomDevice.Channel[i].Status = VC_DEVICE_STREAMING;
                CFE_EVS_SendEvent(VC_DEV_INF_EID, CFE_EVS_INFORMATION,
                        "VC Device streaming channel %u from %s",
                        (unsigned int)i, VC_AppCustomDevice.Channel[i].DevName);
            }
        }
    }
    return returnCode;
}


int32 VC_CleanupDevices(void)
{
    uint32 i = 0;
    int32 returnCode = 0;
    
    for(i=0; i < VC_MAX_DEVICES; i++)
    {
        if(VC_AppCustomDevice.Channel[i].Mode == VC_DEVICE_ENABLED)
        {
            if(VC_AppCustomDevice.Channel[i].Status == VC_DEVICE_STREAMING)
            {    
                if(-1 == VC_Stop_StreamingDevice(i))
                {
                    returnCode = -1;
                }
            }
            
            if(-1 == VC_DisableDevice(i))
            {
                returnCode = -1;
            }
        }
    }
    return returnCode;
}


int32 VC_DisableDevice(uint8 DeviceID)
{
    int32 returnCode = 0;

    if(VC_AppCustomDevice.Channel[DeviceID].Mode != VC_DEVICE_ENABLED)
    {
        CFE_EVS_SendEvent(VC_DEVICE_ERR_EID, CFE_EVS_ERROR,
                            "VC Device for channel %u is not enabled.", (unsigned int)DeviceID);
        returnCode = -1;
        goto end_of_function;
    }

    close(VC_AppCustomDevice.Channel[DeviceID].DeviceFd);

end_of_function:
    return returnCode;
}


boolean VC_Devices_Start(void)
{
    if(VC_Start_Streaming() == -1)
    {
        return FALSE;
    }
    return TRUE;
}


boolean VC_Devices_Stop(void)
{
    if(VC_Stop_Streaming() == -1)
    {
        return FALSE;
    }
    return TRUE;
}


boolean VC_Devices_Init(void)
{
    if(VC_InitDevices() == -1)
    {
        return FALSE;
    }
    return TRUE;
}


boolean VC_Devices_Uninit(void)
{
    if(VC_CleanupDevices() == -1)
    {
        return FALSE;
    }
    return TRUE;
}
