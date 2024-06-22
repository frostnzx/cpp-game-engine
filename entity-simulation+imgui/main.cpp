#include<iostream>
#include<memory>
#include<fstream>
#include<string>

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include <misc/cpp/imgui_stdlib.h>

#define CONFIG_FILE_PATH "./bin/config.txt"

class ShapeProperties {
public:
	std::string name;
	char type; 
	float speedX, speedY , r , g , b;
	bool draw; 
};

int main(int argc, char* argv[]) {

	// define parameters to be read from config
	int wd_width, wd_height , font_size;
	int fontColor[3]; 
	std::string font_address; 
	std::vector<ShapeProperties> shapeProp; // Shape properties for name and velocity

	//sf::Shape* shape = new sf::CircleShape(20);   alternate way
	//std::shared_ptr
	std::vector<std::shared_ptr<sf::Shape>> shapes; // main shapes stores all type cirlce , rect

	// read from config 
	std::ifstream in(CONFIG_FILE_PATH);
	if (!in.is_open()) {
		std::cout << "Could not load config!\n";
		exit(-1);
	}
	while (!in.eof()) {
		std::string param;
		in >> param; 
		if(param == "Window") {
			in >> wd_width >> wd_height; 
		}
		else if (param == "Font") {
			in >> font_address >> font_size; 
			for (int i = 0; i < 3; i++) {
				in >> fontColor[i]; 
			}
		}
		else {
			ShapeProperties TempShape;
			float x, y , radius , width , height ; 
			int r, g, b; 
			in	>> TempShape.name
				>> x >> y
				>> TempShape.speedX
				>> TempShape.speedY
				>> r >> g >> b; 
			TempShape.r = r / 255.0, TempShape.g = g / 255.0, TempShape.b = b / 255.0; 
			if (param == "Circle") {
				in >> radius; 
				TempShape.type = 'C';
				std::shared_ptr<sf::Shape> TempCircle = std::make_shared<sf::CircleShape>(radius);
				TempCircle->setPosition(sf::Vector2f(x, y));
				TempCircle->setFillColor(sf::Color(r , g , b));
				shapes.push_back(TempCircle);
			}
			else {
				in >> width >> height; 
				TempShape.type = 'R';
				std::shared_ptr<sf::Shape> TempRect = std::make_shared<sf::RectangleShape>(sf::Vector2f(width, height));
				TempRect->setPosition(sf::Vector2f(x, y));
				TempRect->setFillColor(sf::Color(r , g , b));
				shapes.push_back(TempRect);
			}
			shapeProp.push_back(TempShape);
		}
	}
	in.close();

	shapes.pop_back(); shapeProp.pop_back();

	// sfml window init
	sf::RenderWindow window(sf::VideoMode(wd_width, wd_height), "SFML_assignment1"); // creating sfml window object
	window.setFramerateLimit(60);

	//create clock for internal timing
	sf::Clock deltaClock;

	// imgui window init
	ImGui::SFML::Init(window); 
	ImGui::GetStyle().ScaleAllSizes(1.0f); // scaling imgui

	// text & fonts
	sf::Font myFont;  // define font obj
	if (!myFont.loadFromFile(font_address)) { // attempt to load font from file
		std::cerr << "Could not load font!\n"; 
		exit(-1);
	}
	int selectedIndex = 0;

	// main game loop
	while (window.isOpen()) {
		sf::Event event; 
		while (window.pollEvent(event)) {
			// pass the event to imgui to be parsed
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}
		// update ImGui window
		ImGui::SFML::Update(window, deltaClock.restart());

		//ImGui::ShowDemoWindow();

		// draw ImGui UI
		ImGui::Begin("Shape Properties");
		
		std::vector<std::string> items; 
		for (int i = 0; i < shapeProp.size(); i++) {
			items.push_back(shapeProp[i].name);
		}
		if (ImGui::BeginCombo("Shape" , items[selectedIndex].c_str())) {
			for (int i = 0; i < items.size(); i++) {
				const bool isSelected = (selectedIndex == i); // taking to confirm if current i-th selectable is being selecting rn
				if (ImGui::Selectable(items[i].c_str(), isSelected)) { // first para is for display string , second is for the gray color thingy
					selectedIndex = i; 
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Checkbox("Draw shape", &shapeProp[selectedIndex].draw);


		if (shapeProp[selectedIndex].type == 'C') {
			float radius = std::dynamic_pointer_cast<sf::CircleShape>(shapes[selectedIndex])->getRadius(); 
			ImGui::SliderFloat("Radius", &radius , 0.0f, 100.0f, "%.3f");
			std::dynamic_pointer_cast<sf::CircleShape>(shapes[selectedIndex])->setRadius(radius);
		}
		else {
			float width = std::dynamic_pointer_cast<sf::RectangleShape>(shapes[selectedIndex])->getSize().x,
				height = std::dynamic_pointer_cast<sf::RectangleShape>(shapes[selectedIndex])->getSize().y; 

			float vec2f[2] = { width , height };
			ImGui::SliderFloat2("Scale", vec2f, 0, 500.0f, "%.3f");
			std::dynamic_pointer_cast<sf::RectangleShape>(shapes[selectedIndex])->setSize(sf::Vector2f(vec2f[0], vec2f[1]));
		}
		float vec2f[2] = {shapeProp[selectedIndex].speedX , shapeProp[selectedIndex].speedY};
		ImGui::SliderFloat2("Velocity", vec2f , -8.0f , 8.0f , "%.3f");
		shapeProp[selectedIndex].speedX = vec2f[0] ; shapeProp[selectedIndex].speedY = vec2f[1]; 

		float c[3] = { shapeProp[selectedIndex].r , shapeProp[selectedIndex].g , shapeProp[selectedIndex].b};
		ImGui::ColorEdit3("Color", c);
		shapeProp[selectedIndex].r = c[0], shapeProp[selectedIndex].g = c[1], shapeProp[selectedIndex].b = c[2]; 
		shapes[selectedIndex]->setFillColor(sf::Color(c[0] * 255, c[1] * 255, c[2] * 255));

		ImGui::InputText("Name", &shapeProp[selectedIndex].name , 32);

		ImGui::End();

		// basic animation move the each frame if it's still in frame
		for (int i = 0; i < shapes.size(); i++) {
			shapes[i]->setPosition(shapes[i]->getPosition().x + shapeProp[i].speedX, shapes[i]->getPosition().y + shapeProp[i].speedY);
			if(shapes[i]->getPosition().x <= 0 || shapes[i]->getPosition().x + shapes[i]->getGlobalBounds().width >= wd_width) {
				shapeProp[i].speedX *= -1.0; 

			}
			if (shapes[i]->getPosition().y <= 0 || shapes[i]->getPosition().y + shapes[i]->getGlobalBounds().height >= wd_height) {
				shapeProp[i].speedY *= -1.0;

			}
		}

		//draw text within the shape



		// here are basic rendering function calls 
		window.clear();
		for (int i = 0;i<shapes.size();i++) {
			if (shapeProp[i].draw) { 
				window.draw(*shapes[i]);
			}
		}
		ImGui::SFML::Render(window); // draw it last so its on top
		window.display(); // swap buffer ( hey we're ready for the next frame ) 
		
	}
}