import os, platform, zipfile

def zipFolder(zip, folder_path, folder_name):
    for fname in os.listdir(folder_path):
        file_path = os.path.join(folder_path, fname)
        file_name = os.path.join(folder_name, fname)
        if os.path.isdir(file_path):
            zipFolder(zip, file_path, file_name)
        zip.write(file_path, file_name)

if platform.system() == "Windows":
    # Increase build number only in windows
    # so run this scrip in windows first, commit and 
    # run scrip in other OSs
    
    import increment_build_number
    
    os.environ["QTDIR"] = "D:\\QtSQK\\Desktop\\Qt\\4.8.1\\msvc2010"
    devenv = os.environ["ProgramFiles"] + "\\Microsoft Visual Studio 10.0\\Common7\\IDE\\devenv.exe"
    nsis = os.environ["ProgramFiles"] + "\\NSIS\\makensis.exe"

    os.system("\""+devenv+"\" ..\\YUVToolkitProject_vs2010.sln /Clean Release /out clean_log.txt")
    os.system("\""+devenv+"\" ..\\YUVToolkitProject_vs2010.sln /Rebuild Release /out build_log.txt")
    os.system("\""+nsis+"\" YUVToolkit.nsi")
    
    version = open("VERSION_1").read()+"."+ \
        open("VERSION_2").read()+"."+ \
        open("VERSION_3").read()+"."+ \
        open("VERSION_4").read()
    
    zip = zipfile.ZipFile("YUVToolkit-"+version+".zip", 'w', zipfile.ZIP_DEFLATED)
    for file in open('file_list.txt').readlines():
        strparts = file.split();
        if strparts[1] == '/r':
            folder_path = strparts[2].replace("\"", "")
            folder_name = os.path.basename(folder_path)
            zipFolder(zip, folder_path, folder_name)
        else:
            file_path = strparts[1].replace("\"", "")
            file_name = os.path.basename(file_path)
            zip.write(file_path, file_name)
    zip.close()


elif platform.system() == "Darwin":
    os.chdir("..")
    os.system("qmake YUVToolkitProject.pro -r -spec macx-g++ -config release")
    os.system("make clean")
    os.system("make")
