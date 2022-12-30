#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "26";
	static const char MONTH[] = "01";
	static const char YEAR[] = "2008";
	static const double UBUNTU_VERSION_STYLE = 8.01;
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 3;
	static const long BUILD = 0;
	static const long REVISION = 4;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 37;
	#define RC_FILEVERSION 0,3,0,4
	#define RC_FILEVERSION_STRING "0, 3, 0, 4\0"
	static const char FULLVERSION_STRING[] = "0.3.0.4";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 1;
	

#endif //VERSION_h
