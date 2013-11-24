__setupPackage__("subjective");

subjective.test = function () {
	this.start = function(videoList) {
		this.videoList = videoList;
		this.current = -1;
		
		yt.actionClose.setEnabled(false);
		yt.enableContextMenu(false);
		yt.actionScore.trigger();
		yt.actionZoom100.trigger();
		
		yt.scoreWindow.onNext.connect(this, "nextVideo");
		yt.scoreWindow.onPrevious.connect(this, "prevousVideo");
		yt.scoreWindow.onFinish.connect(this, "finish");
		
		this.nextVideo();
	};
	
	this.finish = function() {
		yt.scoreWindow.onNext.disconnect(this, "nextVideo");
		yt.scoreWindow.onPrevious.disconnect(this, "prevousVideo");
		yt.scoreWindow.onFinish.disconnect(this, "finish");
		
		yt.actionScore.trigger();
		yt.closeAll();
		
		yt.actionClose.setEnabled(true);
		yt.enableContextMenu(true);
		
		yt.close();
	};
	
	this.openVideo = function() {
		yt.scoreWindow.enableButtons(this.current>0, this.current<this.videoList.length-1, this.current==this.videoList.length-1);
		yt.openFiles(this.videoList[this.current]);
		yt.setWindowTitle("Video " + (this.current+1) + " of " + this.videoList.length);
	};
	
	this.nextVideo = function() {
		if (this.current==this.videoList.length-1) return;
		
		yt.closeAll();
		
		this.current ++;
		this.openVideo();
	};

	this.prevousVideo = function() {
		if (this.current==0) return;
	
		yt.closeAll();
		
		this.current--;
		this.openVideo();
	};
};