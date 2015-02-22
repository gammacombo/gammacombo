#include "ProgressBar.h"

ProgressBar::ProgressBar(OptParser *arg, unsigned int n)
{
  assert(arg);
  _arg = arg;
	_n = n;
	_x = 0;
	_width = 50;
	_resolution = _width;
	_batch = _arg->isAction("pluginbatch") || _arg->isAction("bbbatch");
}

ProgressBar::~ProgressBar()
{}

///
/// Call this from inside the loop.
///
void ProgressBar::progress()
{
	_x++;
	if ( (_x != _n) && (_x % (_n/_resolution+1) != 0) ) return;
	if ( _batch ) progressPercentage();
	else progressBar();
}

///
/// Display the progress as a bar:
/// 100% [==================================================]
///
void ProgressBar::progressBar()
{
	float ratio  =  _x/(float)_n;
	int   c      =  ratio * _width;

	cout << setw(3) << (int)(ratio*100) << "% [";
	for (int x=0; x<c; x++) cout << "=";
	for (int x=c; x<_width; x++) cout << " ";
	cout << "]";
	// this code leaves the progress bar in the output once the loop is done
	//if ( _x<_n ) cout << "\r" << flush;
	//else cout << endl;
	// and this code removes it
	cout << "\r" << flush;
	if ( _x==_n ){
		int offset = TString("100% []").Sizeof();
		for (int x=0; x<c+offset; x++) cout << " ";
		cout << "\r" << flush;
	}
}

///
/// Display the progress as percentage without
/// flushing, so that the logfiles of batch jobs
/// look good.
///
void ProgressBar::progressPercentage()
{
	float ratio  =  _x/(float)_n;
	cout << setw(3) << (int)(ratio*100) << "%" << endl;
}

///
/// Increase the internal count by n steps.
///
void ProgressBar::skipSteps(unsigned int n)
{
	_x += n;
}
