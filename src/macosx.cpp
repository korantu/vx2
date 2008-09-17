#include <string.h>
#include <string>

enum dialog_type {
  DO_LOAD, DO_SAVE
};

#ifdef MACOSX
#define UNDEFINED
#ifdef UNDEFINED
#include <Carbon/Carbon.h>
#endif
std::string GetOpenFileFromUser(dialog_type d_type)
{
#ifdef UNDEFINED
  NavDialogCreationOptions dialogOptions;
  NavDialogRef dialog;
  NavReplyRecord replyRecord;
  //CFURLRef fileAsCFURLRef = NULL;
  FSRef fileAsFSRef;
  OSStatus status;
    CFIndex sz;
    CFRange r;
  
  std::string res;

  // Get the standard set of defaults
  status = NavGetDefaultDialogCreationOptions(&dialogOptions);
  require_noerr( status, CantGetNavOptions );

  // Make the window app-wide modal
  dialogOptions.modality = kWindowModalityAppModal;
  dialogOptions.optionFlags |= kNavDontConfirmReplacement;

  // Create the dialog
  status = (DO_LOAD == d_type)?
    NavCreateGetFileDialog(&dialogOptions, NULL, NULL, NULL, NULL, NULL, &dialog):
    NavCreatePutFileDialog(&dialogOptions,  kNavGenericSignature, kNavGenericSignature, NULL, NULL, &dialog);
  require_noerr( status, CantCreateDialog );

  // Show it
  status = NavDialogRun(dialog);
  require_noerr( status, CantRunDialog );
        
  // Get the reply
  status = NavDialogGetReply(dialog, &replyRecord);
  require( ((status == noErr) || (status == userCanceledErr)), CantGetReply );

  // If the user clicked "Cancel", just bail
  if ( status == userCanceledErr ) goto UserCanceled;

  // Get the file
  status = AEGetNthPtr(&(replyRecord.selection), 1, typeFSRef, NULL, NULL, &fileAsFSRef, sizeof(FSRef), NULL);
  require_noerr( status, CantExtractFSRef );
        
  // Convert it to a CFURL
  // fileAsCFURLRef = CFURLCreateFromFSRef(NULL, &fileAsFSRef);
  //success, lets extraxct info:
  // kdl;
 
 unsigned char buf[100];
  buf[0]=0; //null terminated
  FSRefMakePath( &fileAsFSRef, (UInt8 *)buf, 100); 
  res = std::string((char *)buf, strlen((char *)buf));
  
  if(DO_SAVE == d_type){ //then add file name to the path.
    r.location = 0;
    r.length = CFStringGetLength(replyRecord.saveFileName);
    CFStringGetBytes(
		     replyRecord.saveFileName,
		     r,
		     kCFStringEncodingUTF8,
		     0,     //dont want to deal with uncovertable strings
		     false, //internal representation
		     buf,
		     100,
		     &sz);
    res += "/";
    res += std::string((char *)buf, sz);		     
  };
  
  // Cleanup
 CantExtractFSRef:
 UserCanceled:
  verify_noerr( NavDisposeReply(&replyRecord) );
 CantGetReply:
 CantRunDialog:
  NavDialogDispose(dialog);
 CantCreateDialog:
 CantGetNavOptions:
  return res;
#else
  return "";
#endif //UNDEFINED
};

std::string getFile(){
  return GetOpenFileFromUser(DO_LOAD);
};

std::string putFile(){
  return GetOpenFileFromUser(DO_SAVE);
};
#endif //MACOSX

#ifdef WIN32

#ifdef UNICODE
#undef UNICODE
#endif
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

std::string FileDialog(bool do_save){
OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "FreeSurfer volumes (*.mgz)\0*.mgz\0FreeSurfer surfaces (*.pial, *.pial.asc)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "mgz";

	std::string res = "";
	if(do_save?GetOpenFileName(&ofn):GetOpenFileName(&ofn))
    {
        // Do something usefull with the filename stored in szFileName 
	    res = ofn.lpstrFile;
	}
	return res;
};


std::string getFile(){
	return FileDialog(false);
};

std::string putFile(){
	return FileDialog(true);
};
#endif //WIN32

#ifdef LINUX
//stubs
std::string getFile(){
  return "";
};

std::string putFile(){
  return "";
};

#endif

