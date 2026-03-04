Pipelines that worked 

gst-launch-1.0 libcamerasrc camera-name=/base/axi/pcie@1000120000/rp1/i2c@88000/imx290@1a !   video/x-raw,width=640,height=480,format=NV12,framerate=30/1 !   videoconvert ! autovideosink

Testing

gst-launch-1.0 libcamerasrc camera-name=/base/axi/pcie@1000120000/rp1/i2c@88000/imx290@1a !   video/x-raw,width=640,height=480,format=NV12,framerate=30/1 !   videoconvert ! appsink