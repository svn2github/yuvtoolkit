__setupPackage__("SAMVIQ");

SAMVIQ.test = function () {
	this.start = function(InputList) {
		this.videoList = yt.scoreWindow.shuffleList(InputList, true, false, false);
		this.current = -1;
		
		yt.PopNameInputWindow();
		this.username = yt.getNameInputUserName();
		this.resultfilename = this.username + "_SAMVIQ_result.txt";
		

		yt.enableSubjectiveTestInterface(true);

		yt.setSetting("main/playbackloop", false);
		
		playback_control = function () {
			this.counter = 0;
			this.lastFrameDisplayed = function() {
				yt.closeAll();
			}
		};
		playback = new playback_control()
		yt.lastFrameDisplayed.connect(playback, "lastFrameDisplayed");
		
		yt.scoreWindow.changeButtonName("Replay video", "Next", "Finish");
		
		yt.scoreWindow.onNext.connect(this, "nextVideo");
		yt.scoreWindow.onPrevious.connect(this, "prevousVideo");
		yt.scoreWindow.onFinish.connect(this, "finish");
		
		//enable rate Button, set number and their names
		//enable rate scale, set scale range and level names
		yt.scoreWindow.SetVideoListInCurrentScene(this.videoList[0]);
		videoNumPerScene = this.videoList[0].length;
		yt.scoreWindow.createSliderandButton(videoNumPerScene,0,100,"ref,video1,video2,video3","Excellent,good,fair,poor,bad");
		
		this.nextVideo();
	};
	
	this.finish = function() {
		yt.scoreWindow.onNext.disconnect(this, "nextVideo");
		yt.scoreWindow.onPrevious.disconnect(this, "prevousVideo");
		yt.scoreWindow.onFinish.disconnect(this, "finish");
		
		line = yt.scoreWindow.getCurSliderResults(this.videoList[this.current]);
		yt.scoreWindow.openResultsFile(this.resultfilename, 2);
		yt.scoreWindow.writeResultsFile(line);
		yt.scoreWindow.closeResultsFile();
		
		yt.closeAll();	
		yt.close();
	};
	
	this.openVideo = function() {
		yt.scoreWindow.SetVideoListInCurrentScene(this.videoList[this.current]);
		yt.scoreWindow.initSlider(true);
		yt.scoreWindow.enableButtons(true, false, this.current==this.videoList.length-1);
		yt.openFile(this.videoList[this.current][0]);
		yt.setWindowTitle("Video " + (this.current+1) + " of " + this.videoList.length);
	};
	
	this.nextVideo = function() {
		if (this.current==0)
		{
			line = yt.scoreWindow.getCurSliderResults(this.videoList[this.current]);
			yt.scoreWindow.openResultsFile(this.resultfilename, 0);
			yt.scoreWindow.writeResultsFile(line);
			yt.scoreWindow.closeResultsFile();
		}
		if (this.current==this.videoList.length-1) 
		{
			yt.infoMsg('Message','Already the last video! Please press the Finish Button');
			yt.scoreWindow.enableButtons(true, false, this.current==this.videoList.length-1);
			return;
		}
		
		if (this.current > 0) 
		{
			line = yt.scoreWindow.getCurSliderResults(this.videoList[this.current]);
			yt.scoreWindow.openResultsFile(this.resultfilename, 2);
			yt.scoreWindow.writeResultsFile(line);
			yt.scoreWindow.closeResultsFile();
		}
		
		yt.closeAll();
		
		this.current ++;
		this.openVideo();
	};

	this.prevousVideo = function() {
	
		yt.closeAll();
		
		this.openVideo();
	};
};