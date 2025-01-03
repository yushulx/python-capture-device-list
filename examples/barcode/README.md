# Python Multi-Barcode Scanner
This is a simple barcode scanner implemented with [lite-camera](https://pypi.org/project/lite-camera/) and [dynamsoft-capture-vision-bundle](https://pypi.org/project/dynamsoft-capture-vision-bundle/).

## Prerequisites
- Obtain a [30-day free trial license](https://www.dynamsoft.com/customer/license/trialLicense/?product=dcv&package=cross-platform) for Dynamsoft Barcode Reader. Update the following code in `main.py`:
    
    ```python
    errorCode, errorMsg = LicenseManager.init_license(
        "LICENSE-KEY")
    ```
    

## Installation

```bash
pip install -r requirements.txt
```

## Usage

```bash
python main.py
```

![python multi-barcode scanner](https://www.dynamsoft.com/codepool/img/2025/01/python-multi-barcode-scanner.png)