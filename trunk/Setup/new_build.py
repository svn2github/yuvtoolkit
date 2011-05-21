import os, platform

if platform.system() == "Windows":
    # Increase build number only in windows
    # so run this scrip in windows first, commit and 
    # run scrip in other OSs
    import increment_build_number
    
    os.environ["QTDIR"] = "C:\\QT\\4.7.3"
    devenv = os.environ["ProgramFiles"] + "\\Microsoft Visual Studio 8\\Common7\\IDE\\devenv.exe"
    nsis = os.environ["ProgramFiles"] + "\\NSIS\\makensis.exe"

    os.system("\""+devenv+"\" ..\\YUVToolkitProject_vs2005.sln /Clean Release /out clean_log.txt")
    os.system("\""+devenv+"\" ..\\YUVToolkitProject_vs2005.sln /Rebuild Release /out build_log.txt")
    os.system("\""+nsis+"\" YUVToolkit.nsi")
elif platform.system() == "Darwin":
    os.chdir("..")
    os.system("qmake YUVToolkitProject.pro -r -spec macx-g++ -config release")
    os.system("make clean")
    os.system("make")
