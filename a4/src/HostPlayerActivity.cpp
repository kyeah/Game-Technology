#include "Interpolator.h"
#include "MenuActivity.h"
#include "HostPlayerActivity.h"
#include "OBAnimationManager.h"
#include "Networking.h"
#include "Sounds.h"
#include "common.h"
#include "SelectorHelper.h"

HostPlayerActivity::HostPlayerActivity(OgreBallApplication *app, const char* lobbyName, const char* name, const char* levelName) : BaseMultiActivity(app)
{
  currentLevelName = std::string(levelName, std::strlen(levelName));

  Networking::serverConnect();
  waitingForClientsToLoad = false;
  myId = 0;

  chatEditbox->removeEvent(CEGUI::Editbox::EventTextAccepted);
  chatEditbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
                              CEGUI::Event::Subscriber(&HostPlayerActivity::handleTextSubmitted,this));

  lobbyNamebar->setText(lobbyName);
  lobbyStart->setText("Start");

  lobbyStart->removeEvent(CEGUI::PushButton::EventClicked);
  lobbyStart->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&HostPlayerActivity::startGame, this));

  lobbySelectCharacter->removeEvent(CEGUI::PushButton::EventClicked);
  lobbySelectCharacter->subscribeEvent(CEGUI::PushButton::EventClicked,
                                       CEGUI::Event::Subscriber(&HostPlayerActivity::SwitchToPlayerSelectMenu, this));

  lobbySelectLevel->setVisible(true);

  lobbySelectLevel->removeEvent(CEGUI::PushButton::EventClicked);
  lobbySelectLevel->subscribeEvent(CEGUI::PushButton::EventClicked,
                                   CEGUI::Event::Subscriber(&HostPlayerActivity::SwitchToLevelSelectMenu, this));

  addPlayer(0, name);
  this->lobbyName = std::string(lobbyName, strlen(lobbyName));
}

bool HostPlayerActivity::SwitchToPlayerSelectMenu( const CEGUI::EventArgs &e ) {
  SelectorHelper::type_flag = SelectorHelper::TYPE_MULTI_HOST;
  SelectorHelper::SwitchToPlayerSelectMenu();
}

void HostPlayerActivity::handlePlayerSelected(int i) {
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);
  players[myId]->character = i;

  ServerPacket msg;
  msg.type = SERVER_PLAYER_MESH_CHANGE;
  msg.clientID = myId;
  msg.characterChange = i;

  for(int i = 1; i < MAX_PLAYERS; i++) {
    if(players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }
}

bool HostPlayerActivity::SwitchToLevelSelectMenu( const CEGUI::EventArgs &e ) {
  SelectorHelper::type_flag = SelectorHelper::TYPE_MULTI_CHANGE;
  SelectorHelper::SwitchToLevelSelectMenu();
}

bool HostPlayerActivity::handleLevelSelected( const CEGUI::EventArgs &e ) {
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);
  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  currentLevelName = std::string(levelName.c_str(), levelName.length());

  ServerPacket msg;
  msg.type = SERVER_LEVEL_CHANGE;
  strcpy(msg.msg, levelName.c_str());
  for(int i = 1; i < MAX_PLAYERS; i++) {
    if(players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }

  loadLevel(currentLevelName.c_str());
}

HostPlayerActivity::~HostPlayerActivity(void) {
  close();
  loadLevel(currentLevelName.c_str());
}

void HostPlayerActivity::close(void) {
  ServerPacket msg;
  msg.type = SERVER_CLOSED;
  for(int i = 1; i < MAX_PLAYERS; i++) {
    if(players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }

  Networking::Close();
}

void HostPlayerActivity::start(void) {
  BaseMultiActivity::start();
  loadLevel(currentLevelName.c_str());
}

bool HostPlayerActivity::handleTextSubmitted( const CEGUI::EventArgs &e ) {
  CEGUI::String cmsg = chatEditbox->getText();

  std::stringstream ss;
  ss << players[myId]->name << ": " << cmsg.c_str();
  const char *msg = ss.str().c_str();

  toggleChat();

  chatEditbox->setText("");

  if (cmsg.length()) {
    addChatMessage(msg);

    ServerPacket packet;
    packet.type = SERVER_CLIENT_MESSAGE;
    packet.clientID = myId;
    strcpy(packet.msg, msg);
    for (int i = 1; i < MAX_PLAYERS; i++) {
      if (players[i]) {
        Networking::Send(players[i]->csd, (char*)&packet, sizeof(packet));
      }
    }
  }

  allowKeyPress = false;
}

void HostPlayerActivity::handleLobbyState(void) {

  handleClientEvents();

  // Handle New Connections
  TCPsocket csd_t = SDLNet_TCP_Accept(Networking::server_socket);
  if(csd_t){

    remoteIP = SDLNet_TCP_GetPeerAddress(csd_t);
    if(remoteIP){
      printf("Successfully connected to %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
    }

    PingMessage ping;

    if(SDLNet_TCP_Recv(csd_t, &ping, sizeof(ping)) > 0) {

      if (!ping.isJoining) {
        PingResponseMessage response;
        strcpy(response.lobbyName, lobbyName.c_str());

        response.numPlayers = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
          if (players[i]) response.numPlayers++;
        }

        response.maxPlayers = MAX_PLAYERS;
        Networking::Send(csd_t, (char*)&response, sizeof(response));
        SDLNet_TCP_Close(csd_t);

      } else {

        // Add a new player to the lobby
        if (SDLNet_TCP_AddSocket(Networking::server_socketset, csd_t) == -1) {
          printf("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());

        } else {
          for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!players[i]) {
              addPlayer(i, ping.name);
              players[i]->csd = csd_t;

              // Send ack with its new user ID
              ConnectAck ack;
              ack.id = i;
              strcpy(ack.level, currentLevelName.c_str());
              strcpy(ack.lobbyName, "TEMPORARY SWAGGY P");

              for (int j = 0; j < MAX_PLAYERS; j++) {
                if (players[j]) {
                  ack.ids[j] = 1;
                  strcpy(ack.playerInfo[j].name, players[j]->name);
                  // ack.playerInfo[j].characterChoice = WHATEVER CHOICE;
                } else {
                  ack.ids[j] = 0;
                }
              }

              Networking::Send(csd_t, (char*)&ack, sizeof(ack));

              // Send notifications to rest of players
              ServerPacket packet;
              packet.type = SERVER_CLIENT_CONNECT;
              //strcpy(packet.level, currentLevelName.c_str());
              packet.clientID = i;
              for (int j = 1; j < MAX_PLAYERS; j++) {
                if (i != j && players[j])
                  Networking::Send(players[j]->csd, (char*)&packet, sizeof(packet));
              }
              break;
            }
          }
        }
      }
    }
  }
}

void HostPlayerActivity::handleWaiting() {
  // Wait for clients to send READY messages
}

bool HostPlayerActivity::startGame( const CEGUI::EventArgs& e ) {
  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i] && !players[i]->ready) {
      return true;
    }
  }

  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i])
      togglePlayerReady(i);
  }

  ServerPacket packet;
  packet.type = SERVER_GAME_START;

  players[0]->crossedFinishLine = false;
  for (int j = 1; j < MAX_PLAYERS; j++) {
    if (players[j]) {
      Networking::Send(players[j]->csd, (char*)&packet, sizeof(packet));
      players[j]->crossedFinishLine = false;
      players[j]->viewingPlayer = j;
    }
  }

  inGame = true;
  waitingForClientsToLoad = true;

  loadLevel(currentLevelName.c_str());
  CEGUI::System::getSingleton().setGUISheet(guiSheet);
  guiSheet->addChildWindow(chatWindow);
  CEGUI::MouseCursor::getSingleton().hide();
}

void HostPlayerActivity::loadLevel(const char* name) {
  BaseMultiActivity::loadLevel(name);

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      std::stringstream ss;
      ss << "Player" << i;
      std::string playerEnt = ss.str();
      ss << "node";

      char* playerChoice = SelectorHelper::CharacterToString(players[i]->character);

      players[i]->setBall(new OgreBall(app->mSceneMgr, ss.str(), ss.str(), playerChoice,  0,
                                       app->mPhysics,
                                       app->levelLoader->playerStartPositions[0], btVector3(1,1,1),
                                       btVector3(0,0,0), 16000.0f, 0.5f, btVector3(0,0,0),
                                       &app->levelLoader->playerStartRotations[0]));

      if (i == 0) {
        players[i]->mCameraLookAtNode = app->mCameraLookAtNode;
        players[i]->mCameraNode = app->mCameraNode;
      } else {
        ss << "lookAt";
        players[i]->mCameraLookAtNode = app->mSceneMgr->getRootSceneNode()->createChildSceneNode(ss.str());

        ss << "cam";
        players[i]->mCameraNode = players[i]->mCameraLookAtNode->createChildSceneNode(ss.str());
        players[i]->mCameraNode->setFixedYawAxis(true);
      }

      players[i]->mCameraObj = new CameraObject(players[i]->mCameraLookAtNode, players[i]->mCameraNode,
                                                (Ogre::Vector3)players[i]->getBall()->getPosition(),
                                                app->levelLoader->cameraStartPos);
    }
  }

  app->mCamera->setPosition(Ogre::Vector3(0,0,0));
}

bool HostPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  app->mPhysics->stepSimulation(elapsedTime);

  if (!inGame) {
    handleLobbyState();
    return true;
  } else if (waitingForClientsToLoad) {
    handleWaiting();
    //    return true;  // Comment out for now
  }

  if (countdown != -1 && !menuActive && !ceguiActive) {
    int lastcountdown = countdown;
    countdown = std::max((int)(countdown - elapsedTime), -1);
    if (lastcountdown > 1750 && countdown <= 1750) {
      OBAnimationManager::startAnimation("SpinPopin", readyWindow, 0.8);
    } else if (lastcountdown > 0 && countdown <= 0) {
      OBAnimationManager::startAnimation("Popin", goWindow);
    }
  }

  if (!gameEnded && countdown == -1) {
    timeLeft = std::max(timeLeft - elapsedTime, 0.0f);

    if (timeLeft == 0.0f) {
      handleGameEnd();
    }
  }


  /*
    if (!gameEnded  && countdown == -1) {
    timeLeft = std::max(timeLeft - elapsedTime, 0.0f);

    if (timeLeft == 0.0f) {
    handleGameOver();
    } else if (app->levelLoader->fallCutoff > player->getPosition()[1]) {
    lives--;
    if (lives < 0) {
    handleGameOver();
    } else {
    OBAnimationManager::startAnimation("SpinPopup", livesDisplay);
    loadLevel(currentLevelName.c_str());
    return true;
    }
    }
    }
  */

  // Update HUD
  std::stringstream sst;
  sst << "SCORE: " << score;
  scoreDisplay->setText(sst.str());

  /*
    std:: stringstream livesSS;
    livesSS << lives << (lives != 1 ? " Lives" : " Life");
    livesDisplay->setText(livesSS.str());
  */

  std::stringstream css;
  css << collectibles << "/" << app->levelLoader->numCollectibles;
  collectDisplay->setText(css.str());

  std::stringstream timess;
  int seconds = std::round(timeLeft/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod(timeLeft,1000)/10));

  timess << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  timeDisplay->setText(timess.str());

  // Update tilts
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      Player *player = players[i];
      if (app->levelLoader->fallCutoff > player->getBall()->getPosition()[1]) {
        player->getBall()->setPosition(app->levelLoader->playerStartPositions[0]);
        player->getBall()->setVelocity(btVector3(0,0,0));
      }

      player->currTilt = Interpolator::interpQuat(player->currTiltDelay, elapsedTime,
                                                  tiltDelay,
                                                  player->lastTilt,
                                                  player->tiltDest);
    }
  }

  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = players[i];
    if (player) {

      // Set player's gravity based on the direction they are facing
      Ogre::Vector3 ocam = player->mCameraNode->_getDerivedPosition();
      btVector3 facingDirection = player->getBall()->getPosition() - btVector3(ocam[0], ocam[1], ocam[2]);
      facingDirection[1] = 0;
      facingDirection.normalize();

      btScalar yaw = btVector3(0,0,-1).angle(facingDirection);
      btQuaternion q((facingDirection[0] > 0 ? -yaw : yaw),0,0);
      q.normalize();

      if (player->crossedFinishLine) {
        player->getBall()->getBody()->setGravity(btVector3(0, 1000, 0));
      } else if (countdown == -1) {
        btVector3 tweakedGrav = 1.8*app->mPhysics->getDynamicsWorld()->getGravity()
          .rotate(player->currTilt.getAxis(), -2.5*player->currTilt.getAngle())
          .rotate(q.getAxis(), q.getAngle());

        tweakedGrav[1] /= 1.5;
        player->getBall()->getBody()->setGravity(tweakedGrav);
      }

      // Update Camera Position
      player->mCameraObj->update((Ogre::Vector3)player->getBall()->getPosition(), elapsedTime);

      // Tilt Camera to simulate level tilt
      if (countdown == -1) {
        Ogre::Quaternion oq = Ogre::Quaternion(q.w(), q.x(), q.y(), q.z());
        Ogre::Quaternion noq = Ogre::Quaternion(-q.w(), q.x(), q.y(), q.z());

        Ogre::Real xTilt = player->currTilt.x();
        if (xTilt < 0) xTilt /= 3;
        Ogre::Quaternion notilt = Ogre::Quaternion(-player->currTilt.w(),
                                                   xTilt,
                                                   player->currTilt.y(),
                                                   player->currTilt.z());

        if (!oq.isNaN() && !notilt.isNaN() && !noq.isNaN()) {
          player->mCameraLookAtNode->rotate(oq);
          player->mCameraLookAtNode->rotate(notilt*notilt);
          player->mCameraLookAtNode->rotate(noq);
        }
      }
    }
  }

  app->mCamera->lookAt((Ogre::Vector3)players[0]->getBall()->getPosition() + Ogre::Vector3(0,250,0));

  handleClientEvents();
  updateClients();

  return true;
}

void HostPlayerActivity::handleClientEvents(void) {
  while (SDLNet_CheckSockets(Networking::server_socketset, 1) > 0) {
    for (int i = 1; i < MAX_PLAYERS; i++) {
      if (players[i]) {
        TCPsocket csd = players[i]->csd;
        if (SDLNet_SocketReady(csd)) {
          ClientPacket cmsg;
          ServerPacket closemsg;

          if(SDLNet_TCP_Recv(csd, &cmsg, sizeof(cmsg)) <= 0) {
            if (inGame) {
              app->mPhysics->removeObject(players[i]->getBall());
              app->mSceneMgr->destroyEntity(players[i]->getBall()->getEntity());
              app->mSceneMgr->destroyEntity(players[i]->getBall()->getHeadEntity());
            }

            SDLNet_TCP_DelSocket(Networking::server_socketset, csd);
            SDLNet_TCP_Close(csd);
            players[i] = NULL;

            closemsg.type = SERVER_CLIENT_CLOSED;
            closemsg.clientID = i;
            for (int j = 1; j < MAX_PLAYERS; j++) {
              if (players[j]) {
                Networking::Send(players[j]->csd, (char*)&closemsg, sizeof(closemsg));
              }
            }
          } else {
            switch (cmsg.type) {
            case KEY_PRESSED:
              handleKeyPressed(cmsg.keyArg, cmsg.userID);
              break;
            case KEY_RELEASED:
              handleKeyReleased(cmsg.keyArg, cmsg.userID);
              break;
            case MOUSE_PRESSED:
              if (players[i]->crossedFinishLine) {
                for (int j = 1; j < 3; j++) {
                  int next = (players[i]->viewingPlayer + j) % 4;
                  if (players[next] && !players[next]->crossedFinishLine) {
                    players[i]->viewingPlayer = next;
                    break;
                  }
                }
              }
              break;
            case CLIENT_TOGGLEREADY:
              togglePlayerReady(cmsg.userID);

              closemsg.type = CLIENT_TOGGLEREADY;
              closemsg.clientID = i;
              for (int i = 1; i < MAX_PLAYERS; i++) {
                if (players[i])
                  Networking::Send(players[i]->csd, (char*)&closemsg, sizeof(closemsg));
              }
              break;
            case SERVER_PLAYER_MESH_CHANGE: {
              players[cmsg.userID]->character = cmsg.characterChange;

              ServerPacket msg;
              msg.type = SERVER_PLAYER_MESH_CHANGE;
              msg.clientID = myId;
              msg.characterChange = cmsg.characterChange;

              for(int i = 1; i < MAX_PLAYERS; i++) {
                if(players[i] && i != cmsg.characterChange) {
                  Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
                }
              }
              break;
            }
            case CLIENT_CHAT: {
              addChatMessage(cmsg.msg);

              ServerPacket packet;
              packet.type = SERVER_CLIENT_MESSAGE;
              packet.clientID = i;
              memcpy(packet.msg, cmsg.msg, 512);

              for (int j = 1; j < MAX_PLAYERS; j++) {
                if (i != j && players[j]) {
                  Networking::Send(players[j]->csd, (char*)&closemsg, sizeof(closemsg));
                }
              }
              break;
            }
            case CLIENT_CLOSE:
              if (inGame) {
                app->mPhysics->removeObject(players[i]->getBall());
                app->mSceneMgr->destroyEntity(players[i]->getBall()->getEntity());
                app->mSceneMgr->destroyEntity(players[i]->getBall()->getHeadEntity());
              }

              SDLNet_TCP_DelSocket(Networking::server_socketset, csd);
              SDLNet_TCP_Close(csd);
              players[i] = NULL;

              closemsg.type = SERVER_CLIENT_CLOSED;
              closemsg.clientID = i;
              for (int j = 1; j < MAX_PLAYERS; j++) {
                if (players[j]) {
                  Networking::Send(players[j]->csd, (char*)&closemsg, sizeof(closemsg));
                }
              }
            }
          }
        }
      }
    }
  }
}

void HostPlayerActivity::updateClients(void) {
  ServerPacket msg;

  msg.type = SERVER_UPDATE;
  msg.timeLeft = timeLeft;

  std::deque<GameObject*> objects = app->mPhysics->getObjects();
  for (int i = 0; i < objects.size() && i < 200; i++) {
    msg.objectInfo[i].position = objects[i]->getPosition();
    msg.objectInfo[i].orientation = objects[i]->getOrientation();
  }

  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      msg.camInfo.position = players[players[i]->viewingPlayer]->mCameraNode->_getDerivedPosition();
      msg.camInfo.orientation = players[players[i]->viewingPlayer]->mCameraNode->_getDerivedOrientation();
      msg.camInfo.viewingPlayer = players[i]->viewingPlayer;

      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }
}

//-------------------------------------------------------------------------------------

void HostPlayerActivity::handleCrossedFinishLine( int id ) {
  if (!players[id]->crossedFinishLine) {
    players[id]->crossedFinishLine = true;
    if (id == 0) {
      BaseMultiActivity::handleCrossedFinishLine();
    } else {
      ServerPacket msg;
      msg.type = SERVER_CROSSED_FINISH_LINE;
      Networking::Send(players[id]->csd, (char*)&msg, sizeof(msg));
    }
  }

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i] && !players[i]->crossedFinishLine)
      return;
  }

  handleGameEnd();
}

void HostPlayerActivity::handleGameEnd() {
  BaseMultiActivity::handleGameEnd();

  ServerPacket packet;
  packet.type = SERVER_GAME_END;
  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i])
      Networking::Send(players[i]->csd, (char*)&packet, sizeof(packet));
  }
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::keyPressed( const OIS::KeyEvent &arg )
{
  if (!BaseMultiActivity::keyPressed(arg))
    return handleKeyPressed(arg.key, myId);

  return true;
}

bool HostPlayerActivity::handleKeyPressed( OIS::KeyCode arg, int userId ) {
  Player *player = players[userId];
  if (!player) return false;

  switch(arg){
  case OIS::KC_RETURN:
    if (allowKeyPress) {
      toggleChat();
      chatEditbox->setText("");
    }
    break;
  case OIS::KC_D:
    player->tiltDest *= btQuaternion(0,0,-MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_A:
    player->tiltDest *= btQuaternion(0,0,MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_W:
    player->tiltDest *= btQuaternion(0,-MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_S:
    player->tiltDest *= btQuaternion(0,MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_R:
    break;
  default:
    return false;
  }

  return true;
}
//-------------------------------------------------------------------------------------

bool HostPlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (!BaseMultiActivity::keyReleased(arg))
    return handleKeyReleased(arg.key, myId);

  return true;
}

bool HostPlayerActivity::handleKeyReleased( OIS::KeyCode arg, int userId ) {
  Player *player = players[userId];
  if (!player) return false;

  switch(arg){
  case OIS::KC_D:
    player->tiltDest *= btQuaternion(0,0,MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_A:
    player->tiltDest *= btQuaternion(0,0,-MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_W:
    player->tiltDest *= btQuaternion(0,MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_S:
    player->tiltDest *= btQuaternion(0,-MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  default:
    return false;
  }

  return true;
}

bool HostPlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  if (!BaseMultiActivity::mousePressed(arg, id)) {
    if (players[myId]->crossedFinishLine) {
      for (int i = 1; i < 3; i++) {
        int next = (players[myId]->viewingPlayer + i) % 4;
        if (players[next] && !players[next]->crossedFinishLine) {
          players[myId]->viewingPlayer = next;
          break;
        }
      }
    }
  }

  return true;
}
