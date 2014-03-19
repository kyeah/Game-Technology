#include <iostream>
 #include "src/SoundManager.h"
 
 int main()
 {
   std::cout << "Simple Sound Manager test application !" << std::endl;
 
   SoundManager* soundMgr = SoundManager::createManager();
 
   std::cout << soundMgr->listAvailableDevices();
 
   soundMgr->init();
 
   soundMgr->setAudioPath( (char*) ".\\" );
 
   unsigned int audioId;
 
   // We loop to be able to test the pause function
   soundMgr->loadAudio( "test.ogg", &audioId, true);
 
 /*   
    // Set our LISTENER Location (i.e. the ears)
    // The listener is what hears the sounds emitted by audio sources.
    // Note: you only have ONE set of ears - i.e. there is only ONE listener.
    soundMgr->setListenerPosition( mPlayerSceneNode->getWorldPosition(),
        Ogre::Vector3::ZERO, mPlayerSceneNode->getOrientation() );
 
    // Set the Audio SOURCE location/position AND by default PLAY the audio.
    // Note how we reference the correct audio source by using 'mAudioSource'.
 
    soundMgr->setSound( audioId, mSomeObjectSceneNode->getWorldPosition(),
        Ogre::Vector3::ZERO, Ogre::Vector3::ZERO, 1000.0f, true, true, 1.0f );
 */
 
    // Play an Audio source - force a restart from the begining of the buffer.
    // That means, re-play the media file from the begining.
    soundMgr->playAudio( audioId, true );
 
    printf("Audio playing.  Type Enter.\n");
    getchar(); // To hear the sound until the end.
 
    soundMgr->pauseAudio( audioId );
 
    printf("Audio paused.  Type Enter.\n");
    getchar();
 
 //   soundMgr->resumeAudio( audioId );
    soundMgr->resumeAllAudio( );
 
    printf("Audio playing. Type Enter\n");
    getchar();
 
    // Force an audio source to stop playing.
    soundMgr->stopAudio( audioId );
 
   // Release an audio source when we are done with it.
   // This isn't required if you destruct the AudioEngine - as it cleans itself up.
   // But, you *SHOULD* do this when, for example, your SceneObject is no longer
   // in the scene for some reason. In other words, if you don't need this source
   // anymore then free up the resource. Remember, you are limited in audio sources.
   soundMgr->releaseAudio( audioId );
 
   delete soundMgr;
 
   return 0;
 }
