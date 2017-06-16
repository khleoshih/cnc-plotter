import processing.serial.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
Serial serial;

int posX, posY;
int targetX, targetY;
int state;
int i;
String input;
String[] gcode;
Pattern pattern = Pattern.compile(".*X.*Y.*");
Pattern patternX = Pattern.compile("X\\s*[0-9\\-]+\\.?[0-9]*");
Pattern patternY = Pattern.compile("Y\\s*[0-9\\-]+\\.?[0-9]*");
int MM_TO_STEPS = 200;
int PEN_UP_ANGLE = 60;
int PEN_DOWN_ANGLE = 110;
void setup(){
  size(400,250);
  serial = new Serial(this, "COM8", 9600);
  targetX = 0;
  targetY = 0;
}

void draw(){
  background(0);
  textSize(20); 
  text("X Position: "+posX, 50, 30);
  text("Y Position: "+posY, 50, 55);
  text("X Target: "+targetX, 50, 80);
  text("Y Target: "+targetY, 50, 105);
  textSize(14);
  text("Press 'c' for zero offset", 50, 130);
  text("Press Arrow keys to operate", 50, 150);
  text("Press 'z' for pen up", 50, 170);
  text("Press 'x' for pen down", 50, 190);
  text("Press 'g' to load a gcode file", 50, 210);
}

void keyPressed(){
  if (keyCode == UP){
    println("up");
    targetX += 500;
    serial.write(Integer.toString(500));
    serial.write('x');
  }
  else if (keyCode == DOWN){
    println("down");
    targetX -= 500;
    serial.write(Integer.toString(-500));
    serial.write('x');
  }
  else if (keyCode == LEFT){
    println("left");
    targetY -= 500;
    serial.write(Integer.toString(-500));
    serial.write('y');
  }
  else if (keyCode == RIGHT){
    println("right");
    targetY += 500;
    serial.write(Integer.toString(500));
    serial.write('y');
  }
  else if ((key == 'z')||(key == 'Z')){
    println("pen up");
    state = PEN_UP_ANGLE;
    serial.write(Integer.toString(state));
    serial.write('p');
  }
  else if ((key == 'x')||(key == 'X')){
    println("pen down");
    state = PEN_DOWN_ANGLE;
    serial.write(Integer.toString(state));
    serial.write('p');
  }
  else if ((key == 'g')||(key == 'G')){
    println("load");
    gcode = null;
    i = 0;
    File file = null;
    selectInput("Select a file to process:", "fileSelected", file);
  }
  else if ((key == 'c')||(key == 'C')){
    println("zero offset");
    targetX = 0;
    targetY = 0;
    serial.write('o');
  }
}

void fileSelected(File selection) {
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
  } else {
    println("User selected " + selection.getAbsolutePath());
    gcode = loadStrings(selection.getAbsolutePath());
    if (gcode == null) return;
    transmit_gcode();
  }
}

void serialEvent (Serial myPort) 
{
  try {
    String myString = myPort.readStringUntil('\n');
 
    if (myString != null) 
    {
      myString = trim(myString);
      println(myString);
      if (myString.startsWith("GcodeDone")){
        println("Received");
        transmit_gcode();
      }
      else if(myString.startsWith("Position:")){
        String[] s = split(myString, ':');
        String[] pos = split(s[1], ',');
        posX = int(pos[0]);
        posY = int(pos[1]);
      }
      /*String[] pos = split(myString, ',');
      posX = int(pos[0]);
      posY = int(pos[1]);*/
    }
  }
  catch(RuntimeException e) {
    e.printStackTrace();
  }
}

void transmit_gcode(){
  int len = gcode.length;
  if (i >= len) return;
  gcode[i] = gcode[i].replaceAll("%","");
  gcode[i] = gcode[i].replaceAll("\\(([^)]+)\\)","");
  //gcode[i] = gcode[i].trim();
  if (gcode[i].length() == 0){
    i++;
    transmit_gcode();
  }
  else{
    println(gcode[i]);
    Matcher match = pattern.matcher(gcode[i]);
    if (match.matches()){
      Matcher matchX = patternX.matcher(gcode[i]);
      Matcher matchY = patternY.matcher(gcode[i]);
      matchX.find();
      matchY.find();
      targetX = (int)(Float.parseFloat(matchX.group().replaceAll("[^0-9.\\-]",""))*MM_TO_STEPS);
      targetY = (int)(Float.parseFloat(matchY.group().replaceAll("[^0-9.\\-]",""))*MM_TO_STEPS);
    }
    serial.write(gcode[i] + 'g');
    //println(gcode[i]);
    i++;
  }
}