#include <Carbon/Carbon.h>
#include <string.h>
#include <string>

std::string GetOpenFileFromUser(void)
{
  NavDialogCreationOptions dialogOptions;
  NavDialogRef dialog;
  NavReplyRecord replyRecord;
  CFURLRef fileAsCFURLRef = NULL;
  FSRef fileAsFSRef;
  OSStatus status;

  // Get the standard set of defaults
  status = NavGetDefaultDialogCreationOptions(&dialogOptions);
  require_noerr( status, CantGetNavOptions );

  // Make the window app-wide modal
  dialogOptions.modality = kWindowModalityAppModal;

  // Create the dialog
  status = NavCreateGetFileDialog(&dialogOptions, NULL, NULL, NULL, NULL, NULL, &dialog);
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

  // Cleanup
 CantExtractFSRef:
 UserCanceled:
  verify_noerr( NavDisposeReply(&replyRecord) );
 CantGetReply:
 CantRunDialog:
  NavDialogDispose(dialog);
 CantCreateDialog:
 CantGetNavOptions:
  // kdl;
  unsigned char buf[100];
  buf[0]=0; //null terminated
  FSRefMakePath( &fileAsFSRef, (UInt8 *)buf, 100); 
  return std::string((char *)buf, strlen((char *)buf));
  
};

std::string getFile(){
  return GetOpenFileFromUser();
};

