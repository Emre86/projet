package client;

import java.io.*;
import opencard.core.service.*;
import opencard.core.terminal.*;
import opencard.core.util.*;
import opencard.opt.util.*;




public class TheClient {

	private PassThruCardService servClient = null;
	boolean DISPLAY = true;
	boolean loop = true;

	static final byte CLA								= (byte)0x00;
	static final byte P1								= (byte)0x00;
	static final byte P2								= (byte)0x00;
	static final byte UPDATECARDKEY						= (byte)0x14;
	static final byte UNCIPHERFILEBYCARD				= (byte)0x13;
	static final byte CIPHERFILEBYCARD					= (byte)0x12;
	static final byte CIPHERANDUNCIPHERNAMEBYCARD		= (byte)0x11;
	static final byte READFILEFROMCARD					= (byte)0x10;
	static final byte WRITEFILETOCARD					= (byte)0x09;
	static final byte UPDATEWRITEPIN					= (byte)0x08;
	static final byte UPDATEREADPIN						= (byte)0x07;
	static final byte DISPLAYPINSECURITY				= (byte)0x06;
	static final byte DESACTIVATEACTIVATEPINSECURITY	= (byte)0x05;
	static final byte ENTERREADPIN						= (byte)0x04;
	static final byte ENTERWRITEPIN						= (byte)0x03;
	static final byte READNAMEFROMCARD					= (byte)0x02;
	static final byte WRITENAMETOCARD					= (byte)0x01;


	/* NOMBRE DE PACKET */
	static byte P1_NB_TOTAL ;
	static byte P2_N_PACKET ;
	
	static final byte P_LECTURE  = (byte) 0x60 ;
	static final byte P_DECODE = (byte) 0x70; 
	static final byte P_ENCODE = (byte) 0x80;
	
	
	/* FIN DE NOMBRE DE PACKET */
	public TheClient() {
		try {
			SmartCard.start();
			System.out.print( "Smartcard inserted?... " ); 

			CardRequest cr = new CardRequest (CardRequest.ANYCARD,null,null); 

			SmartCard sm = SmartCard.waitForCard (cr);

			if (sm != null) {
				System.out.println ("got a SmartCard object!\n");
			} else
				System.out.println( "did not get a SmartCard object!\n" );

			this.initNewCard( sm ); 

			SmartCard.shutdown();

		} catch( Exception e ) {
			System.out.println( "TheClient error: " + e.getMessage() );
		}
		java.lang.System.exit(0) ;
	}

	private ResponseAPDU sendAPDU(CommandAPDU cmd) {
		return sendAPDU(cmd, true);
	}

	private ResponseAPDU sendAPDU( CommandAPDU cmd, boolean display ) {
		ResponseAPDU result = null;
		try {
			result = this.servClient.sendCommandAPDU( cmd );
			if(display)
				displayAPDU(cmd, result);
		} catch( Exception e ) {
			System.out.println( "Exception caught in sendAPDU: " + e.getMessage() );
			java.lang.System.exit( -1 );
		}
		return result;
	}


	/************************************************
	 * *********** BEGINNING OF TOOLS ***************
	 * **********************************************/


	private String apdu2string( APDU apdu ) {
		return removeCR( HexString.hexify( apdu.getBytes() ) );
	}


	public void displayAPDU( APDU apdu ) {
		System.out.println( removeCR( HexString.hexify( apdu.getBytes() ) ) + "\n" );
	}


	public void displayAPDU( CommandAPDU termCmd, ResponseAPDU cardResp ) {
		System.out.println( "--> Term: " + removeCR( HexString.hexify( termCmd.getBytes() ) ) );
		System.out.println( "<-- Card: " + removeCR( HexString.hexify( cardResp.getBytes() ) ) );
	}


	private String removeCR( String string ) {
		return string.replace( '\n', ' ' );
	}

	/* MY TOOLS */
	private byte[] myGetBytes(String msg,int n){
		byte[] msgbytes ;
		int tmp = -1;
		int padding = 0;
		tmp = msg.length() % n ;
		if (tmp != 0 ){
			padding = n - ( msg.length()  - (( msg.length() / n) * n ) ) ;
			//System.out.println("longueur du padding :"+padding);
			for (int i=0; i< padding; i++){
				msg = msg + " ";
			}
		}
		
		msgbytes = new byte[msg.length()];
		msgbytes = msg.getBytes();
		return msgbytes ;
	}
	
	
	private String myReadFile (String file){
		String chaine="";	
		try{
			InputStream ips=new FileInputStream(file); 
			InputStreamReader ipsr=new InputStreamReader(ips);
			BufferedReader br=new BufferedReader(ipsr);
			String ligne;
			while ((ligne=br.readLine())!=null){
				chaine+=ligne;
			}
			br.close(); 
		}		
		catch (Exception e){
			System.out.println(e.toString());
		}

		return chaine ;
	}
	
	private void myWriteFile (String file,String data){
	
		try {
			FileWriter fw = new FileWriter (file);
			BufferedWriter bw = new BufferedWriter (fw);
			PrintWriter fichierSortie = new PrintWriter (bw); 
			fichierSortie.println (data); 
			fichierSortie.close();
			System.out.println("Le fichier " + file + " a été créé!"); 
		}
		catch (Exception e){
			System.out.println(e.toString());
		}	
		
	}
	/* END MY TOOLS */
	
	/******************************************
	 * *********** END OF TOOLS ***************
	 * ****************************************/


	private boolean selectApplet() {
		boolean cardOk = false;
		try {
			CommandAPDU cmd = new CommandAPDU( new byte[] {
				(byte)0x00, (byte)0xA4, (byte)0x04, (byte)0x00, (byte)0x0A,
				    (byte)0xA0, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x62, 
				    (byte)0x03, (byte)0x01, (byte)0x0C, (byte)0x06, (byte)0x01
			} );
			ResponseAPDU resp = this.sendAPDU( cmd );
			if( this.apdu2string( resp ).equals( "90 00" ) )
				cardOk = true;
		} catch(Exception e) {
			System.out.println( "Exception caught in selectApplet: " + e.getMessage() );
			java.lang.System.exit( -1 );
		}
		return cardOk;
	}


	private void initNewCard( SmartCard card ) {
		if( card != null )
			System.out.println( "Smartcard inserted\n" );
		else {
			System.out.println( "Did not get a smartcard" );
			System.exit( -1 );
		}

		System.out.println( "ATR: " + HexString.hexify( card.getCardID().getATR() ) + "\n");


		try {
			this.servClient = (PassThruCardService)card.getCardService( PassThruCardService.class, true );
		} catch( Exception e ) {
			System.out.println( e.getMessage() );
		}

		System.out.println("Applet selecting...");
		if( !this.selectApplet() ) {
			System.out.println( "Wrong card, no applet to select!\n" );
			System.exit( 1 );
			return;
		} else 
			System.out.println( "Applet selected" );

	
		mainLoop();
	}

	
	
	
	void updateCardKey() {
	}


	void uncipherFileByCard() {
		ResponseAPDU resp = null ;
		//sun.misc.BASE64Encoder encoder = new sun.misc.BASE64Encoder();
		int lcc = 120 ;
		int p1_nb_total = 0 ;
		byte lc = (byte) 120 ;
		byte [] header = { CLA , UNCIPHERFILEBYCARD, P1 , P2 , lc } ;
		byte [] cmd ;
		byte[] result ;
		byte [] fichierbytes ;
		byte [] unciphered ;
	
		String fichier = myReadFile("C:/Documents and Settings/emre/Bureau/Fichier_cipher.txt");
		System.out.println("LENGHT "+fichier.length());
		fichierbytes = myGetBytes(fichier,120);
		p1_nb_total = fichierbytes.length/120 ;
	
		cmd = new byte [ lcc + 6 ] ;
		cmd[cmd.length - 1] = (byte) 120 ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length );
		result = new byte [ fichierbytes.length ] ;
		for ( int i = 0 ; i < p1_nb_total ; i++ ){		
			System.arraycopy ( fichierbytes , (i * lcc ) , cmd , 5 , lcc ) ;
			
			resp = this.sendAPDU( new CommandAPDU ( cmd ), DISPLAY ) ;
			unciphered = resp.getBytes();
			
			//System.out.print("\nunciphered is:\n" + encoder.encode(result) + "\n");
			System.arraycopy(unciphered , 0 , result , (i * lcc ) , unciphered.length - 2 );
			//System.out.print("\nunciphered is:\n" + encoder.encode(result) + "\n");
		}
		String test = new String(result);
		System.out.println(test);
		
	}


	void cipherFileByCard() {
		ResponseAPDU resp = null ;
		//sun.misc.BASE64Encoder encoder = new sun.misc.BASE64Encoder();
		
		int lcc = 120 ;
		int p1_nb_total = 0 ;
		byte lc = (byte) 120 ;
		byte [] header = { CLA , CIPHERFILEBYCARD, P1 , P2 , lc } ;
		byte [] cmd ;
		byte[] result ;
		byte [] fichierbytes ;
		byte [] ciphered ;
	
		/*
		String fichier = "Le chapitre des performances est un peu particulier. ";
		fichier = fichier + "mais également celles qui sont  ressenties . Ces dernières  " ;
		fichier = fichier + "qui n’ont l’air de rien mais qui, mises bout à bout, améliorent  " ;
		fichier = fichier + "la sensation de fluidité. Exemples : enregistrer la page d’accueil  " ;
		fichier = fichier + "classique sur le disque dur, pour ne pas l’appeler depuis Internet, ou  " ;
		fichier = fichier + "encore charger en priorité l’onglet actif en cas de restauration d’une session.  " ;

		fichier = fichier + "Sur les performances, Nitot insiste sur les performances JavaScript. " ;
		fichier = fichier + "À travers des tests tels que SunSpider, Kraken ou encore V8, on obtient  " ;
		fichier = fichier + "des performances de 3 à 6 fois meilleurs que sur Firefox 3.6.  " ;
		fichier = fichier + "Ces performances continueront d’être travaillées, version après version, " ;
		fichier = fichier + "mais elles atteignent désormais un degré tel que tous les navigateurs  " ;
		fichier = fichier + "récents peuvent être décrits comme vraiment rapides. " ;
		*/
		
		// fichier = readKeyboard();
		String fichier = myReadFile("C:/Documents and Settings/emre/Bureau/Fichier.txt");
		String result_fichier ;
		
		fichierbytes = myGetBytes(fichier,120);
		p1_nb_total = fichierbytes.length/120 ; 
		
		cmd = new byte [ lcc + 6 ] ;
		result = new byte [ fichierbytes.length ] ;
		//encoder.encode(fichierbytes) ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length );
		cmd[cmd.length - 1] = (byte) 120 ;
		
		for ( int i = 0 ; i < p1_nb_total ; i++ ){		
			System.arraycopy ( fichierbytes , (i * lcc ) , cmd , 5 , lcc ) ;
			
			resp = this.sendAPDU( new CommandAPDU ( cmd ), DISPLAY ) ;
			ciphered = resp.getBytes();
		
			System.arraycopy(ciphered , 0 , result , (i * lcc ) , ciphered.length - 2 );
			
		}
		//encoder.encode(result);
		result_fichier = new String (result);
		//System.out.println("TAILLE DU FICHIER EN SORTIE "+result_fichier.length());
		myWriteFile("C:/Documents and Settings/emre/Bureau/Fichier_cipher.txt",result_fichier);
		//myReadFileByte("C:/Documents and Settings/emre/Bureau/Fichier_cipher.txt");
	}


	void cipherAndUncipherNameByCard() {
		ResponseAPDU resp ;
		
		sun.misc.BASE64Encoder encoder = new sun.misc.BASE64Encoder();
	    byte[] response;
	    byte[] unciphered; 
	    
		String name = readKeyboard();
		byte[] challengeDES = myGetBytes(name,8);
		/* ENCODAGE */
		
	   	System.out.println("\nchallenge:\n" + encoder.encode(challengeDES) + "\n");
	    byte[] result = new byte[challengeDES.length];
		byte[] cmd = new byte[challengeDES.length + 6]; 
		cmd[0] =	CLA ;
		cmd[1] =    CIPHERANDUNCIPHERNAMEBYCARD ;
		cmd[2] =    P_ENCODE ;
		cmd[3] =    P2 ;
		cmd[4] =   (byte) challengeDES.length ;
		
		System.arraycopy ( challengeDES , 0 , cmd , 5 , challengeDES.length) ;
		cmd[challengeDES.length + 5 ] = (byte) challengeDES.length ;
		
	    resp = this.sendAPDU( new CommandAPDU ( cmd ), DISPLAY ) ;
		byte[] ciphered = resp.getBytes();
		
		System.arraycopy(ciphered , 0 , result , 0 , ciphered.length - 2 );
		
		/* DECODAGE */
		
		System.out.println("\nciphered is:\n" + encoder.encode(result) + "\n");
		
		byte[] result2 = new byte[result.length];
		cmd = new byte[result.length + 6]; 
		cmd[0] =	CLA ;
		cmd[1] =    CIPHERANDUNCIPHERNAMEBYCARD ;
		cmd[2] =    P_DECODE ;
		cmd[3] =    P2 ;
		cmd[4] =   (byte) result.length ;
		
		System.arraycopy ( result , 0 , cmd , 5 , result.length) ;
		cmd[result.length + 5 ] = (byte) result.length ;
		
	    resp = this.sendAPDU( new CommandAPDU ( cmd ), DISPLAY ) ;
		ciphered = resp.getBytes();
			
		System.arraycopy(ciphered , 0 , result2 , 0 , ciphered.length - 2 );	
		
		System.out.print("\nunciphered is:\n" + encoder.encode(result2) + "\n");
	
	}
	

	void readFileFromCard() {
		ResponseAPDU result = null ;
		int p1_nb_total = 0;
		int i = 0 ;
		byte [] header = { CLA , READFILEFROMCARD , P_LECTURE , P2 ,(byte)0 } ;
		byte [] cmd    = new byte [header.length] ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		result = sendAPDU ( new CommandAPDU(cmd), DISPLAY );
		if( !this.apdu2string( result ).endsWith( "90 00" ) )
			System.out.println("Entrer le code pin de lecture");
		byte[] bytes = result.getBytes();
	    String msg = "";
	    for( i=0; i<bytes.length-2;i++)
		    msg += new StringBuffer("").append((char)bytes[i]);
	    System.out.println(msg);
	
		header[2] = P1 ;
		p1_nb_total = (int) bytes[0];
		System.out.println(" "+ (int) bytes[0] );
		for ( int j = 0 ; j < p1_nb_total ; j++){ 
			header[3] = (byte) j ; 
			System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
			result = sendAPDU ( new CommandAPDU(cmd), DISPLAY );
			if( !this.apdu2string( result ).endsWith( "90 00" ) )
				System.out.println("Entrer le code pin de lecture");
			bytes = result.getBytes();
			msg = "";
			for( i=0; i<bytes.length-2;i++)
				msg += new StringBuffer("").append((char)bytes[i]);
			System.out.print(msg);
		}
		System.out.println("");
	}


	void writeFileToCard() {
		ResponseAPDU resp = null ;
		
		int lcc = 255 ;
		int p1_nb_total = 0 ;
		byte lc = (byte) 255 ;
		byte [] header = { CLA , WRITEFILETOCARD , P1 , P2 , lc } ;
		byte [] cmd ;
		byte [] fichierbytes ;
	
		/*
		String fichier = "Le chapitre des performances est un peu particulier. ";
		fichier = fichier + "mais également celles qui sont  ressenties . Ces dernières  " ;
		fichier = fichier + "qui n’ont l’air de rien mais qui, mises bout à bout, améliorent  " ;
		fichier = fichier + "la sensation de fluidité. Exemples : enregistrer la page d’accueil  " ;
		fichier = fichier + "classique sur le disque dur, pour ne pas l’appeler depuis Internet, ou  " ;
		fichier = fichier + "encore charger en priorité l’onglet actif en cas de restauration d’une session.  " ;

		fichier = fichier + "Sur les performances, Nitot insiste sur les performances JavaScript. " ;
		fichier = fichier + "À travers des tests tels que SunSpider, Kraken ou encore V8, on obtient  " ;
		fichier = fichier + "des performances de 3 à 6 fois meilleurs que sur Firefox 3.6.  " ;
		fichier = fichier + "Ces performances continueront d’être travaillées, version après version, " ;
		fichier = fichier + "mais elles atteignent désormais un degré tel que tous les navigateurs  " ;
		fichier = fichier + "récents peuvent être décrits comme vraiment rapides. " ;
		*/
		
		//String fichier = readKeyboard();
		
		String fichier = myReadFile("C:/Documents and Settings/emre/Bureau/Fichier.txt");
		
		
		
		fichierbytes = myGetBytes(fichier,255);
		p1_nb_total = fichierbytes.length/255 ; 
		if ( p1_nb_total > 127 ){
			System.out.println("Fichier Volumineux ne peut tenir sur une carte a puce");
		}
	
		P1_NB_TOTAL = (byte) (fichierbytes.length/255);
		
		header[2]=P1_NB_TOTAL ;
		
		for ( int i = 0 ; i < p1_nb_total ; i++ ){
			cmd = new byte [ lcc + 5 ] ;
			header[3] = (byte) i ;
			
			System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		
			System.arraycopy ( fichierbytes , (i * 255) , cmd , header.length ,  (short)( lc & (short) 0x00ff ) ) ;
			
			resp = sendAPDU ( new CommandAPDU(cmd), DISPLAY );	
			if ( this.apdu2string(resp).equals("90 00")){
				System.out.println("Code Correct");
			}
			if ( this.apdu2string (resp).equals("63 01")){
				System.out.println("ERREUR");
			}	
		}
	}
	


	void updateWritePIN() {
		ResponseAPDU resp = null ;
		String pin = readKeyboard();
		if (pin.length()!=4){
			System.out.println("Code PIN ERREUR LENGTH ");
		}
		byte [] data = pin.getBytes() ;
		byte lc = (byte) data.length;
		byte [] cmd = new byte [ lc + 5 ] ;
		byte [] header = { CLA , UPDATEWRITEPIN	 , P1 , P2 , lc } ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		System.arraycopy ( data , 0 , cmd , header.length , lc ) ;
		resp = sendAPDU ( new CommandAPDU(cmd), DISPLAY );	
		if( this.apdu2string( resp ).equals( "90 00" ) ){
				System.out.println("Code Correct");
		} 
		else{
			if ( this.apdu2string (resp).equals("63 01")){
				System.out.println("Code pin requis");
			}
			else {
				System.out.println("ERROR");
			}
		}
	}


	void updateReadPIN() {
		ResponseAPDU resp = null ;
		String pin = readKeyboard();
		if (pin.length()!=4){
			System.out.println("Code PIN ERREUR LENGTH ");
		}
		byte [] data = pin.getBytes() ;
		byte lc = (byte) data.length;
		byte [] cmd = new byte [ lc + 5 ] ;
		byte [] header = { CLA , UPDATEREADPIN	 , P1 , P2 , lc } ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		System.arraycopy ( data , 0 , cmd , header.length , lc ) ;
		resp = sendAPDU ( new CommandAPDU(cmd), DISPLAY );	
		if( this.apdu2string( resp ).equals( "90 00" ) ){
				System.out.println("Code Correct");
		} 
		else{
			if ( this.apdu2string (resp).equals("63 01")){
				System.out.println("Code pin requis");
			}
			else {
				System.out.println("ERROR");
			}
		}
	}


	void displayPINSecurity() {
		ResponseAPDU resp = null ;
		byte[] bytes ;
		String msg="" ;
		byte [] header = { CLA , DISPLAYPINSECURITY , P1 , P2 , (byte) 0};
		resp = sendAPDU ( new CommandAPDU(header), DISPLAY) ;
		if ( this.apdu2string( resp ).endsWith("90 00")){
				System.out.println("Correct");
		}
		else{
				System.out.println("Incorrect");
		}
		bytes = resp.getBytes();
	    for(int i=0; i<bytes.length-2;i++)
		    msg += new StringBuffer("").append((char)bytes[i]);
	    System.out.println(msg);
	}


	void desactivateActivatePINSecurity() {
		ResponseAPDU resp = null ;
		byte [] header = { CLA , DESACTIVATEACTIVATEPINSECURITY , P1 , P2 };
		/*byte [] cmd = new byte [4];
		System.arraycopy ( header , 0 , cmd , 0 , header.length );*/
		resp = sendAPDU ( new CommandAPDU(header), DISPLAY) ;
		if ( this.apdu2string( resp ).equals("90 00")){
				System.out.println("Correct");
		}
		else{
				System.out.println("Incorrect");
		}
	}


	void enterReadPIN() {
		ResponseAPDU resp = null ;
		String pin = readKeyboard();
		if (pin.length()!=4){
			System.out.println("Code PIN ERREUR ");
		}
		byte [] data = pin.getBytes() ;
		byte lc = (byte) data.length;
		byte [] cmd = new byte [ lc + 5 ] ;
		byte [] header = { CLA , ENTERREADPIN , P1 , P2 , lc } ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		System.arraycopy ( data , 0 , cmd , header.length , lc ) ;
		resp = sendAPDU ( new CommandAPDU(cmd), DISPLAY );	
		if( this.apdu2string( resp ).equals( "90 00" ) ){
				System.out.println("Code Correct");
		} 
		else {
				System.out.println("Code Incorrect");
		}

	}


	void enterWritePIN() {
		ResponseAPDU resp = null ;
	
		String pin = readKeyboard();
		if (pin.length()!=4){
			System.out.println("Code PIN ERREUR ");
		}
		byte [] data = pin.getBytes() ;
		byte lc = (byte) data.length;
		byte [] cmd = new byte [ lc + 5 ] ;
		byte [] header = { CLA , ENTERWRITEPIN , P1 , P2 , lc } ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		System.arraycopy ( data , 0 , cmd , header.length , lc ) ;
		resp = sendAPDU ( new CommandAPDU(cmd), DISPLAY );	
		if( this.apdu2string( resp ).equals( "90 00" ) ){
				System.out.println("Code Correct");
		} 
		else {
				System.out.println("Code Incorrect");
		}
		
	}


	void readNameFromCard() {
		ResponseAPDU result = null ;
	
		byte [] header = { CLA , READNAMEFROMCARD , P1 , P2 ,(byte)0 } ;
		byte [] cmd    = new byte [header.length] ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		result = sendAPDU ( new CommandAPDU(cmd), DISPLAY );
		if( !this.apdu2string( result ).endsWith( "90 00" ) )
			System.out.println("Entrer le code pin de lecture");
		byte[] bytes = result.getBytes();
	    String msg = "";
	    for(int i=0; i<bytes.length-2;i++)
		    msg += new StringBuffer("").append((char)bytes[i]);
	    System.out.println(msg);
	}


	void writeNameToCard() {
		ResponseAPDU result = null ;
		String name = readKeyboard();
		byte [] data = name.getBytes() ;
		byte lc = (byte) data.length;
		byte [] cmd = new byte [ lc + 5 ] ;
		byte [] header = { CLA , WRITENAMETOCARD , P1 , P2 , lc } ;
		System.arraycopy ( header , 0 , cmd , 0 , header.length ) ;
		System.arraycopy ( data , 0 , cmd , header.length , lc ) ;
		result = sendAPDU ( new CommandAPDU(cmd), DISPLAY );
		if( !this.apdu2string( result ).equals( "90 00" ) )
			System.out.println("Entrer le code pin d ecriture");
		
	}


	void exit() {
		loop = false;
	}


	void runAction( int choice ) {
		switch( choice ) {
			case 14: updateCardKey(); break;
			case 13: uncipherFileByCard(); break;
			case 12: cipherFileByCard(); break;
			case 11: cipherAndUncipherNameByCard(); break;
			case 10: readFileFromCard(); break;
			case 9: writeFileToCard(); break;
			case 8: updateWritePIN(); break;
			case 7: updateReadPIN(); break;
			case 6: displayPINSecurity(); break;
			case 5: desactivateActivatePINSecurity(); break;
			case 4: enterReadPIN(); break;
			case 3: enterWritePIN(); break;
			case 2: readNameFromCard(); break;
			case 1: writeNameToCard(); break;
			case 0: exit(); break;
			default: System.out.println( "unknown choice!" );
		}
	}


	String readKeyboard() {
		String result = null;

		try {
			BufferedReader input = new BufferedReader( new InputStreamReader( System.in ) );
			result = input.readLine();
		} catch( Exception e ) {}

		return result;
	}


	int readMenuChoice() {
		int result = 0;

		try {
			String choice = readKeyboard();
			result = Integer.parseInt( choice );
		} catch( Exception e ) {}

		System.out.println( "" );

		return result;
	}


	void printMenu() {
		System.out.println( "" );
		System.out.println( "14: update the DES key within the card" );
		System.out.println( "13: uncipher a file by the card" );
		System.out.println( "12: cipher a file by the card" );
		System.out.println( "11: cipher and uncipher a name by the card" );
		System.out.println( "10: read a file from the card" );
		System.out.println( "9: write a file to the card" );
		System.out.println( "8: update WRITE_PIN" );
		System.out.println( "7: update READ_PIN" );
		System.out.println( "6: display PIN security status" );
		System.out.println( "5: desactivate/activate PIN security" );
		System.out.println( "4: enter READ_PIN" );
		System.out.println( "3: enter WRITE_PIN" );
		System.out.println( "2: read a name from the card" );
		System.out.println( "1: write a name to the card" );
		System.out.println( "0: exit" );
		System.out.print( "--> " );
	}


	void mainLoop() {
		while( loop ) {
			printMenu();
			int choice = readMenuChoice();
			runAction( choice );
		}
	}


	public static void main( String[] args ) throws InterruptedException {
		new TheClient();
	}


}
