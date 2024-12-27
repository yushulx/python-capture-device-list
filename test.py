import device

camera_names = device.getDeviceList()
print(camera_names)

camera = device.PyCamera()

if camera.open(0):
    print("Camera is opened")

    mediaTypes = camera.listMediaTypes()
    print(mediaTypes)
