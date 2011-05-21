build_file = "VERSION_4"
build_nr = int(open(build_file).read())+1
open(build_file, "w").write(str(build_nr))