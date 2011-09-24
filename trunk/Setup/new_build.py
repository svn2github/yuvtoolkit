import os, platform, zipfile

if platform.system() == "Windows":
    # Increase build number only in windows
    # so run this scrip in windows first, commit and 
    # run scrip in other OSs
    
    import increment_build_number
    
    os.environ["QTDIR"] = "D:\\QtSQK\\Desktop\\Qt\\4.7.4\\msvc2008"
    devenv = os.environ["ProgramFiles"] + "\\Microsoft Visual Studio 9.0\\Common7\\IDE\\devenv.exe"
    nsis = os.environ["ProgramFiles"] + "\\NSIS\\makensis.exe"

    os.system("\""+devenv+"\" ..\\YUVToolkitProject_vs2008.sln /Clean Release /out clean_log.txt")
    os.system("\""+devenv+"\" ..\\YUVToolkitProject_vs2008.sln /Rebuild Release /out build_log.txt")
    os.system("\""+nsis+"\" YUVToolkit.nsi")
    
    version = open("VERSION_1").read()+"."+ \
        open("VERSION_2").read()+"."+ \
        open("VERSION_3").read()+"."+ \
        open("VERSION_4").read()
    
    zip = zipfile.ZipFile("YUVToolkit-"+version+".zip", 'w', zipfile.ZIP_DEFLATED)
    for file in open('file_list.txt').readlines():
        file_path = file.split()[1].replace("\"", "")
        file_name = os.path.basename(file_path)
        zip.write(file_path, file_name)
    zip.close()


elif platform.system() == "Darwin":
    os.chdir("..")
    os.system("qmake YUVToolkitProject.pro -r -spec macx-g++ -config release")
    os.system("make clean")
    os.system("make")
