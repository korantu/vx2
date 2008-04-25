#include <Carbon/Carbon.h>
#include <string.h>
#include <string>

enum dialog_type {
  DO_LOAD, DO_SAVE
};


std::string GetOpenFileFromUser(dialog_type d_type)
{
  NavDialogCreationOptions dialogOptions;
  NavDialogRef dialog;
  NavReplyRecord replyRecord;
  CFURLRef fileAsCFURLRef = NULL;
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
};

std::string getFile(){
  return GetOpenFileFromUser(DO_LOAD);
};

std::string putFile(){
  return GetOpenFileFromUser(DO_SAVE);
};

