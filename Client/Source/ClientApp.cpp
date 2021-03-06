#include "ClientApp.h"

#include <iostream>
using namespace std;

/// Startup
ClientApp::ClientApp(){
	setWindowTitle("Moba Client");

	m_client.onDataReceived.connect(MAKE_SLOT_LOCAL(ClientApp, onClientData));
	m_client.onConnected.connect(MAKE_SLOT_LOCAL(ClientApp, onClientConnect));

	m_myHero = NULL;
};

/// When game is created
void ClientApp::onCreate(){
	m_renderer = Renderer::createAutomaticRenderer(&getWindow());
	m_view.setRect(0,0,1024,768);

	secondClock = 0.f;

	dirt.loadFromFile("dirt.png");

	m_ui = RocketContext::create("ui", Vec2i(1024,768));
	UIElementScroll::registerWithlibRocket();
	m_ui->onEvent.connect(MAKE_SLOT_LOCAL(ClientApp, rocketEvent));
	m_ui->loadFont("DroidSansFallback.ttf");
	m_chat = m_ui->showDocument("chat.ui");
	m_ui->showDocument("hud.ui");

	String account_name = "127.0.0.1";
	ScopedFile fp("../../ip.txt", IODevice::TextRead);
	if(fp.canRead()){
		TextStream ts(&fp);
		account_name = ts.getLine();
	}

	m_client.connect(account_name, 8002, 100);
};

/// Called when rocket sends events
void ClientApp::rocketEvent(String event){
	if(event == "chat.send"){
		String message = ((Rocket::Controls::ElementFormControlInput*)m_chat->getElementById("inputbox"))->GetValue().CString();
		((Rocket::Controls::ElementFormControlInput*)m_chat->getElementById("inputbox"))->SetValue("");

		if(message.empty())return;

		Packet pck;
		pck << (Uint32)Client::CHAT_MESSAGE;
		pck << message;
		m_client.send(pck);
	}
	else if(event == "hud.shop"){
		m_ui->showDocument("shop.ui");
	}
};

/// Game events
void ClientApp::onEvent(Event &event){
	m_ui->processEvent(event);

	if(event.type == Event::MouseButtonPressed){
		if(event.mouseButton.button == Mouse::Left){
			

			Vec2f mouse = getWindow().convertCoords(Vec2i(event.mouseButton.x, event.mouseButton.y) , m_view);

			Packet pck;
			pck << (Uint32)Client::DROP_BOMB;
			pck << mouse;
			m_client.send(pck);
		}
		if(event.mouseButton.button == Mouse::Right){
			Vec2f mouse = getWindow().convertCoords(Vec2i(event.mouseButton.x, event.mouseButton.y) , m_view);

			if(m_myHero){
				m_myHero->targetPosition = mouse;
				m_myHero->autoMoving = true;

				Packet p;
				p << (Uint32)Client::HERO_TRAVEL_POSITION;
				p << m_myHero->id;
				p << m_myHero->targetPosition;
				m_client.send(p);
			}
		}
	}
	if(event.type == Event::MouseWheelMoved){

		if(event.mouseWheel.delta > 0){
			m_view.zoom(0.9f);
		}
		else if(event.mouseWheel.delta <= 0){
			m_view.zoom(1.1f);
		}
	}


	if(event.type == Event::KeyPressed){
		if(event.key.code == Keyboard::D){
			if(m_myHero){

				m_myHero->direction.x = 1;
				

				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}

		}
		if(event.key.code == Keyboard::A){
			if(m_myHero){
				m_myHero->direction.x = -1;


				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}

		}
		if(event.key.code == Keyboard::W){
			if(m_myHero){
				m_myHero->direction.y = -1;


				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}

		}
		if(event.key.code == Keyboard::S){
			if(m_myHero){
				
				m_myHero->direction.y = 1;

				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}

		}
	}
	else if(event.type == Event::KeyReleased){
		if(event.key.code == Keyboard::D){	
			if(m_myHero){
				m_myHero->direction.x = 0;

				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}
		}
		if(event.key.code == Keyboard::A){			
			if(m_myHero){
				m_myHero->direction.x = 0;

				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}
		}
		if(event.key.code == Keyboard::S){			
			if(m_myHero){
				m_myHero->direction.y = 0;

				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}
		}
		if(event.key.code == Keyboard::W){			
			if(m_myHero){
				m_myHero->direction.y = 0;

				Packet p;
				p << (Uint32)Client::HERO_DIRECTION_REQUEST;
				p << m_myHero->id;
				p << m_myHero->direction;
				m_client.send(p);
			}
		}
	}
};

/// Rendering
void ClientApp::onRender(){
	m_renderer->clearBuffers();

	m_renderer->setView(m_view);

	//draw some ground
	Sprite s;
	s.setTexture(dirt);
	s.resize(52,52);

	for(float x = 0; x < 3000; x += 52){
		for(float y = 0; y < 3000; y+=52){
			s.setPosition(x,y);
			m_renderer->draw(s);
		}
	}


	for(unsigned int i = 0; i < m_heroList.size(); i++){
		if(m_heroList[i] == m_myHero)
			m_renderer->drawDebugCircle(m_heroList[i]->position, 20, Vec2f(), Color(155,0,0,255));
		else
			m_renderer->drawDebugCircle(m_heroList[i]->position, 20, Vec2f(), Color(155,155,0,255));

		Text t;
		t.setPosition(m_heroList[i]->position);
		String content = m_heroList[i]->nick  + "[" + String::number(m_heroList[i]->health / m_heroList[i]->maxHealth * 100) + "]\nKills: " + String::number(m_heroList[i]->kills) + "   Deaths: " + String::number(m_heroList[i]->deaths);
		if(m_heroList[i]->dead){
			content.append("\nRespawn: " + String::number(m_heroList[i]->respawnTimeLeft));
		}
		if(m_myHero && m_heroList[i]->teamid != m_myHero->teamid){
			t.setColor(Color(255,0,30, 160));
		}
		t.setString(content);
		m_renderer->draw(t);
	}
	View v;
	v.setRect(0,0,1024,768);
	m_renderer->setView(v);
	m_renderer->drawRocketContext(m_ui);

	if(m_myHero){
		Text t;
		t.setString("Gold: " + String::number(m_myHero->gold));
		m_renderer->draw(t);
	}

	/*ParticleSystem p;
	p.createSampleSparkles("");
	m_renderer->draw(p);*/

	m_renderer->display();
};

/// Updating the game
void ClientApp::onUpdate(Time time){
	m_client.update(1);
	m_ui->update();

	secondClock += time.asSeconds();

	for(unsigned int i = 0; i < m_heroList.size(); i++){
		if(!m_heroList[i]->dead){

			if(m_heroList[i]->autoMoving){
				// interpolate to target
				

				float angle = Math::computeAngle( m_heroList[i]->position, m_heroList[i]->targetPosition);
				Vec2f dir(cos(angle), sin(angle));
				m_heroList[i]->position += dir * m_heroList[i]->movementSpeed * time.asSeconds();

				if(Math::distance(m_heroList[i]->position, m_heroList[i]->targetPosition) <= m_heroList[i]->movementSpeed * time.asSeconds() ){
					m_heroList[i]->autoMoving = false;
					m_heroList[i]->direction = Vec2f(0,0);
					m_heroList[i]->position = m_heroList[i]->targetPosition;
				}

			}
			else{
				m_heroList[i]->position += m_heroList[i]->direction * m_heroList[i]->movementSpeed * time.asSeconds();

			}

		}

		if(m_heroList[i] == m_myHero)
			m_view.setCenter(m_heroList[i]->position);

		if(m_heroList[i]->dead)
			m_heroList[i]->respawnTimeLeft -= time.asSeconds();
	}

	if(secondClock > 1.f){
		//Update second

		for(unsigned int i = 0; i < m_heroList.size(); i++){
			m_heroList[i]->gold += 1;
		}

		secondClock = 0.f;
	}
};


/// Called when the client connected
void ClientApp::onClientConnect(NetworkClient* client){
	String account_name = "Grimshaw";
	ScopedFile fp("../../account.txt", IODevice::TextRead);
	if(fp.canRead()){
		TextStream ts(&fp);
		account_name = ts.getLine();
	}

	cout<<"Account: "<<account_name<<endl;

	Packet p;
	p << (Uint32)Client::AUTH_REQUEST;
	p << account_name;	
	client->send(p);
};

/// Called when there is new data to read
void ClientApp::onClientData(NetworkClient* , NetworkPacket* packet){
	Packet pck = packet->getData();
	Uint32 packet_id;
	pck >> packet_id;

	switch(packet_id){
		case Server::AUTH_SUCESSFULL:
		{
			cout<<"Server authorized login."<<endl;
		}break;
		case Server::HERO_INFO:
			{
				String nick;
				Int16 id, tid;
				Vec2f pos;
				float movement;
				float hp, maxHp;

				pck >> id >> tid >> pos >> nick >> movement >> hp >> maxHp;

				Hero *hero = new Hero();
				hero->id = id;
				hero->teamid = tid;
				hero->position = pos;
				hero->nick = nick;
				hero->movementSpeed = movement;
				hero->health = hp;
				hero->maxHealth = maxHp;

				m_heroList.push_back(hero);

			}break;

		case Server::HERO_IDENTITY:
			{
				Int16 id;
				pck >> id;

				for(unsigned int i = 0; i < m_heroList.size(); i++){
					if(m_heroList[i]->id == id){
						//found our hero
						m_myHero = m_heroList[i];
					}
				}
			}break;
		case Server::HERO_DAMAGE:
			{
				Int16 id, damage;
				pck >> id >> damage;

				Hero* hero = getHeroById(id);
				if(hero){
					hero->health -= damage;
				}
			}break;
		case Server::HERO_RESPAWN:
			{
				Int16 id;
				pck >> id;

				Hero* hero = getHeroById(id);
				if(hero){
					// respawn him
					pck >> hero->position;
					pck >> hero->health;

					hero->dead = false;
				}
			}break;
		case Server::HERO_DEATH:
			{
				Int16 id, source, respawn, gold;
				pck >> id >> respawn >> gold >> source;

				Hero* hero = getHeroById(id);
				if(hero){
					hero->dead = true;
					hero->respawnTime = respawn;
					hero->respawnTimeLeft = respawn;
					hero->deaths ++;

					Hero* killer = getHeroById(source);
					if(killer){
						killer->kills ++;
						cout<<killer->nick << " has killed "<<hero->nick<<"!"<<endl;
						killer->gold += gold;
					}
				}
			}break;
		case Server::HERO_RECOVER:
			{
				Int16 id, damage;
				pck >> id >> damage;

				Hero* hero = getHeroById(id);
				if(hero){
					hero->health += damage;
				}
			}break;
		case Server::HERO_DIRECTION_UPDATE:
			{
				Int16 id;
				pck >> id;

				for(unsigned int i = 0; i < m_heroList.size(); i++){
					if(m_heroList[i]->id == id){
						//found our hero
						pck >> m_heroList[i]->direction;
						m_heroList[i]->autoMoving = false;
					}
				}

				
			}break;
		case Server::HERO_TRAVEL_UPDATE:
			{
				Int16 id;
				pck >> id;

				for(unsigned int i = 0; i < m_heroList.size(); i++){
					if(m_heroList[i]->id == id){
						//found our hero
						pck >> m_heroList[i]->targetPosition;
						m_heroList[i]->autoMoving = true;
					}
				}


			}break;
		case Server::GLOBAL_CHAT_MESSAGE:
			{
				String message;
				Int16 from;

				pck >> from >> message;

				Hero* hero = getHeroById(from);
				if(hero){
					cout << hero->nick << " said: "<<message<<endl;

					RocketElement* container = m_chat->getElementById("container");
					if(container){
						message = " <p style=\"color: red;\">"  + hero->nick + ":</p> " + message;
						RocketElement* entry = (RocketElement*)Rocket::Core::Factory::InstanceElement(container, "entry", "entry", Rocket::Core::XMLAttributes());
						entry->SetInnerRML(message.c_str());
						container->AppendChild(entry);

						RocketElement* scroller = m_chat->getElementById("scroller");
						if(scroller){
							scroller->SetScrollTop(scroller->GetScrollTop() + entry->GetClientHeight());
						}
						
					}

				}
				

			}break;
	}
};

/// Find a hero by its id
Hero* ClientApp::getHeroById(int id){
	for(unsigned int i = 0; i < m_heroList.size(); i++){
		if(m_heroList[i]->id == id)return m_heroList[i];
	}
	return NULL;
};