Tested under FFmpeg 0.7-rc1
under windows

c:\Dev\vsvars32-v8-all.cmd
c:\Dev\msys.cmd

On other OS than windows, remove --enable-shared --disable-static

./configure --enable-memalign-hack --enable-shared --disable-static --disable-decoders --disable-encoders --disable-demuxers --disable-hwaccels --disable-muxers --disable-network --disable-parsers --disable-bsfs --disable-protocols --disable-indevs --disable-outdevs --disable-devices --disable-filters --enable-decoder=h264 --enable-decoder=mpeg1video --enable-decoder=mpeg2video --enable-decoder=flv --enable-decoder=mpeg4 --enable-decoder=tiff --enable-decoder=cinepak --enable-decoder=mjpeg --enable-decoder=wmv1 --enable-decoder=wmv2 --enable-decoder=wmv3 --enable-decoder=mpegvideo --enable-decoder=rawvideo --enable-demuxer=h264 --enable-demuxer=avi --enable-demuxer=rawvideo --enable-demuxer=avisynth --enable-demuxer=flv --enable-demuxer=matroska --enable-demuxer=m4v --enable-demuxer=mjpeg --enable-demuxer=mpegts --enable-demuxer=mpegtsraw --enable-demuxer=mpegvideo --enable-demuxer=swf --enable-demuxer=vc1 --enable-demuxer=mov --enable-protocol=file --prefix=..

make

make install

cd ../lib 

cp libavcodec.a avcodec.a
ranlib avcodec.a
mv avcodec.a avcodec.lib

cp libavcodec.a avcodec.a
ranlib avcodec.a
mv avcodec.a avcodec.lib

cp libavcodec.a avcodec.a
ranlib avcodec.a
mv avcodec.a avcodec.lib

cp libavcodec.a avcodec.a
ranlib avcodec.a
mv avcodec.a avcodec.lib

cd ..

mv lib lib_win32