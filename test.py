import device
import time
camera_names = device.getDeviceList()
print(camera_names)

camera = device.PyCamera()

if camera.open(0):
    print("Camera is opened")

    mediaTypes = camera.listMediaTypes()
    print(mediaTypes)

    camera.setResolution(640, 480)

    for i in range(3):
        frame = camera.captureFrame()
        if frame is not None:
            width = frame[0]
            height = frame[1]
            size = frame[2]
            data = frame[3]
            filename = str(i) + ".jpg"
            camera.saveJpeg(filename, width, height, data)

        time.sleep(1)

    camera.release()
