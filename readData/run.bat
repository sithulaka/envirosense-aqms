@echo off
:: Check for Python installation
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Python is not installed!
    echo Installing Python...
    start "" "https://www.python.org/downloads/"
    echo Please install Python and run this script again
    pause
    exit
)

:: Install requirements
echo Installing requirements...
pip install -r requirements.txt

:: Run the conversion
echo Running data conversion...
python readData.py

:: Create EXE version
echo Creating EXE version...
pip install pyinstaller
pyinstaller --onefile --windowed readData.py

echo Done!
echo You can now:
echo 1. Use the Python script (readData.py)
echo 2. Or use the EXE version in the 'dist' folder
pause
