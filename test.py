import device
import cv2

def select_camera(last_index):
    number = 0
    hint = "Select a camera (0 to " + str(last_index) + "): "
    try:
        number = int(input(hint))
        # select = int(select)
    except Exception:
        print("It's not a number!")
        return select_camera(last_index)

    if number > last_index:
        print("Invalid number! Retry!")
        return select_camera(last_index)

    return number


def open_camera(index):
    cap = cv2.VideoCapture(index)
    return cap

def main():
    # print OpenCV version
    print("OpenCV version: " + cv2.__version__)

    # Get camera list
    device_list = device.getDeviceList()
    index = 0

    for name in device_list:
        print(str(index) + ': ' + name)
        index += 1

    last_index = index - 1

    if last_index < 0:
        print("No device is connected")
        return

    # Select a camera
    camera_number = select_camera(last_index)
    
    # Open camera
    cap = open_camera(camera_number)

    if cap.isOpened():
        width = cap.get(3) # Frame Width
        height = cap.get(4) # Frame Height
        print('Default width: ' + str(width) + ', height: ' + str(height))

        while True:
            
            ret, frame = cap.read();
            cv2.imshow("frame", frame)

            # key: 'ESC'
            key = cv2.waitKey(20)
            if key == 27:
                break

        cap.release() 
        cv2.destroyAllWindows() 

if __name__ == "__main__":
    main()