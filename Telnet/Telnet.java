import java.io.*;
import java.net.*;

public class Telnet extends Thread {

    Socket s;
    BufferedReader inputConsole ;
    BufferedReader inputNetwork ;
    PrintStream outputNetwork ;
    PrintStream outputConsole ;
    String login ;
    
    public Telnet (String [] args) throws Exception{

        if( args.length != 3 ){
            System.out.println ("java telnet <host> <port> <login>");
            System.exit(1);
        }
        else {
            initStreams(args);
            start();
            listenConsole();
        }
    }

    void initStreams(String[] args) throws Exception {
	System.out.println(" HHHH "+ args[0]);
        s = new Socket(args[0],Integer.parseInt(args[1]));
	login = args[2];
	System.out.println(" AAAAAA ");
        outputConsole = System.out;
        outputNetwork = new PrintStream(s.getOutputStream());
        inputConsole = new BufferedReader(new InputStreamReader (System.in));
        inputNetwork = new BufferedReader(new InputStreamReader (s.getInputStream()));
        
        
    }
    
    public void run(){
        listenNetwork();
    }
    
    void listenConsole() throws Exception{
        while(true){
	    
            String msg = inputConsole.readLine();
	    msg = login+": "+msg;
            outputNetwork.println(msg);
        }
    }
    
    void listenNetwork(){
        while(true) try {
            String msg = inputNetwork.readLine();
            outputConsole.println(msg);
	    if (msg.equals("MANY USERS, TRY AFTER")){
		break;
	    }
        } catch (Exception e){}
	System.exit(0);
    }
    
    public static void main(String [] args) throws Exception {
        new Telnet(args);
    }    
}
        
    