call c:\Dev\vsvars32-v8-basic.cmd

lib /machine:x86 /def:src\libavutil\avutil-50.def /out:lib\avutil.lib
lib /machine:x86 /def:src\libavcodec\avcodec-52.def /out:lib\avcodec.lib
lib /machine:x86 /def:src\libavformat\avformat-52.def /out:lib\avformat.lib
lib /machine:x86 /def:src\libavdevice\avdevice-52.def /out:lib\avdevice.lib
lib /machine:x86 /def:src\libfilter\libfilter-1.def /out:lib\libfilter.lib
lib /machine:x86 /def:src\libswscale\swscale-0.def /out:lib\swscale.lib