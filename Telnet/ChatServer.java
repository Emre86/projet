import java.io.* ;
import java.net.*; 


public class ChatServer extends Thread {

    static final int NBUSERSMAX = 2 ;
    Socket s;
    BufferedReader input ;
    PrintStream output;
   

    static PrintStream[] outputs = new PrintStream[NBUSERSMAX];
    static int  nbUsers=0;
    static int us ;
    public static void main(String[]args) throws Exception{
	if (args.length != 1 )
	    System.out.println("Usage: java ChatServer <port>");
	else {
	    ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));
		System.out.println("Serveur Demarre");
	    while(true){
		Socket s = ss.accept();
		new ChatServer(s);
	    }
	}
    }
	
    public ChatServer(Socket s){
	this.s=s;
	start();
    }
    
    public void run(){
	try {
	    initStreams();
	    mainLoop();
	}
	catch (Exception e){}
    }

    void initStreams(){
	try {
	    input = new BufferedReader(new InputStreamReader(s.getInputStream()));
	    output = new PrintStream(s.getOutputStream());
	    for ( us = 0 ; us < NBUSERSMAX ; us++){
		if (outputs[us] == null){
		    outputs[us] = output;
		    nbUsers++;
		    break;
		}
	    }
	    if ( us == NBUSERSMAX ){
		output.println("Essayer ulterieurement");
	    }
		    
	}
	catch (IOException e){
	    System.out.println("Erreur sur les sockets");
	}
    }
	
    void mainLoop() throws Exception {
		int id = us;
		while(true){
			System.out.println("WAIT ");
			String msg = input.readLine();
			if (msg == null ){
			outputs[id].close();
			outputs[id] = null;	
			System.out.println("Client "+ id +" Deconnecté");
			nbUsers--;
			break;
			}
	    
			for ( int i = 0 ; i < NBUSERSMAX ; i++){
			//if ((i != id) && (outputs[i] != null))
			if (outputs[i] != null)
				outputs[i].println(msg);
			// }
			}
			msg = "";
		}
    }
}