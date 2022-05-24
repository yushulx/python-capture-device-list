# List Capture Devices for Python OpenCV on Windows
**OpenCV** does not have an API for listing capture devices. The sample shows how to create a Python extension to invoke DirectShow C++ APIs for enumerating capture devices.

## Environment
* [Microsoft Windows SDK][0]
* Python 2.7.9 / Python 3.6.5
* OpenCV 3.3.0

## How to Build the CPython Extension
- Create a source distribution:
    
    ```bash
    python setup.py sdist
    ```

- distutils:
    
    ```bash
    python .\setup_distutils.py build
    ```

- scikit-build:
    
    ```bash
    pip wheel . --verbose
    ```

## Test 
```bash
python test.py
```
![camera list in Python](https://raw.githubusercontent.com/yushulx/python-capture-device-list/master/screenshot/python-list-device.PNG)

## Blog
[Listing Multiple Cameras for OpenCV-Python on Windows][1]

[0]:https://en.wikipedia.org/wiki/Microsoft_Windows_SDK
[1]:https://www.dynamsoft.com/codepool/multiple-camera-opencv-python-windows.html
