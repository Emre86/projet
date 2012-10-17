package com.android.andexplorer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

public class AndExplorerActivity extends Activity implements
		OnItemClickListener, Constante {
	private ListView listViewFile;
	private SimpleAdapter simpleAdapter;
	private ArrayList<HashMap<String, String>> listItem;
	private Button button1;
	private Button button2;
	private String sRacine = "/sdcard";
	private String sLastDir = sRacine;
	private FileOnSystem fileOnSystem;
	private File currentFileDir = new File(sRacine);
	ArrayList<HashMap<String, String>> resultForContextMenu ;
	
	
	
	private String FileNameRestart = "AndroidExplorerCurrentDir";

	private String ERREUR = "AndroidExplorer error ";
	
	private String sContextMenuFileName="" ;

	/* TABLEAU AUDIO */

	private String sTypesMusique[] = { sMP3, sM4A, sAAC, sFLAC, sMID, sXMF,
			sMXMF, sRTTTL, sRTX, sOTA, sIMY, sOGG };

	/* TABLEAU VIDEO */

	private String sTypesVideo[] = { s3GP, sMP4, sWEBM, sTS, sMKV };

	/* TABLEAU IMAGE */

	private String sTypesImage[] = { sPNG, sGIF, sJPG, sJPEG, sBMP };

	/* TABLEAU TEXT */
	
	private String sTypesRead[]  =  {sTXT,sCSV,sXML};
	

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.row);
		setContentView(R.layout.main);
		listViewFile = (ListView) findViewById(R.id.list);
		listItem = new ArrayList<HashMap<String, String>>();

		/* Recuperation du PWD lors de la derniere utilisation de l application*/
		
		try {
			FileInputStream fis = openFileInput(FileNameRestart);
			int iFileNameRestart = 0;
			sLastDir = "";

			while ((iFileNameRestart = fis.read()) != -1) {
				sLastDir = sLastDir + ((char) iFileNameRestart);
			}
			currentFileDir = new File(sLastDir);
			if (!currentFileDir.exists()) {
				sLastDir = sRacine;
			}
			fis.close();

		} catch (FileNotFoundException e) {
			Log.i(ERREUR, "FILE NOT FOUND " + FileNameRestart);
			e.printStackTrace();
		} catch (IOException e) {
			Log.i(ERREUR, "FILE IOEXCEPTION " + FileNameRestart);
			e.printStackTrace();
		}

		/* Lancement de l AsyncTask pour le traitement */
		
		fileOnSystem = new FileOnSystem();
		fileOnSystem.execute(new String[] { sLastDir });

		/* Mise en place de l'interface graphique */
		
		simpleAdapter = new SimpleAdapter(this.getBaseContext(), listItem,
				R.layout.row, new String[] { "img", "directory" }, new int[] {
						R.id.img, R.id.directory });

		listViewFile.setAdapter(simpleAdapter);
		listViewFile.setOnItemClickListener(this);

		/* Boutton pour remonter dans l'arborescence */
		
		button1 = (Button) findViewById(R.id.button1);
		button1.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				if (!currentFileDir.toString().equals(sRacine)) {
					Toast.makeText(AndExplorerActivity.this,
							"Current " + currentFileDir, Toast.LENGTH_SHORT)
							.show();
					fileOnSystem = new FileOnSystem();
					fileOnSystem.execute(new String[] { currentFileDir
							.getParent() });
					currentFileDir = currentFileDir.getParentFile();
				} else {
					Toast.makeText(AndExplorerActivity.this,
							"Current " + currentFileDir, Toast.LENGTH_SHORT)
							.show();
				}

			}
		});

		/* Boutton pour revenir au repertoire racine / */
		
		button2 = (Button) findViewById(R.id.button2);
		button2.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				Toast.makeText(AndExplorerActivity.this, "text",
						Toast.LENGTH_SHORT).show();
				fileOnSystem = new FileOnSystem();
				fileOnSystem.execute(new String[] { sRacine });
				currentFileDir = new File(sRacine);
			}
		});
	}

	/* Enregistrement du PWD lors de l'arret de l'application */
	
	public void onDestroy() {
		try {
			FileOutputStream fos = openFileOutput(FileNameRestart,
					Context.MODE_PRIVATE);
			fos.write(currentFileDir.toString().getBytes());
			fos.close();
		} catch (FileNotFoundException e) {
			Log.i(ERREUR, "FILE NOT FOUND " + FileNameRestart);
			e.printStackTrace();
		} catch (IOException e) {
			Log.i(ERREUR, "IOEXCEPTION FILE " + FileNameRestart);
			e.printStackTrace();
		}
		super.onDestroy();
	}

	/*Action effectuer lors du clic sur un item */
	
	@SuppressWarnings("unchecked")
	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int position,
			long arg3) {
		
		HashMap<String, String> map;
		map = (HashMap<String, String>) simpleAdapter.getItem(position);
		Set<String> st = map.keySet();
		Iterator<String> itr = st.iterator();
		String first = itr.next();
		String second = itr.next();
		String img = map.get(first);
		String dir = map.get(second);
		int iImg = Integer.parseInt(img);
		

		if (iImg == iFOL) {
			File fileDir = new File(dir);
			if (fileDir.canRead()) {
				currentFileDir = fileDir;
				fileOnSystem = new FileOnSystem();
				fileOnSystem.execute(new String[] { dir });
			} else {
				/* Si repertoire non lisible message d'erreur 
				 * envoye par un toast a l'utilisateur
				 * */
				Toast.makeText(getBaseContext(),
						" Repertoire interdit d'acces ", Toast.LENGTH_SHORT)
						.show();
			}
		}
		if (iImg == iMUS) {
			FileActivity(dir, "audio/*") ;
		}
		if (iImg == iVID) {
			FileActivity(dir, "video/*") ;
		}
		if (iImg == iICO) {
			FileActivity(dir, "image/*") ;
		}
		if (iImg == iTXT) {
			FileActivity(dir, "text/*")  ;
		}
		/*
		if (iImg == iZIP){
			FileActivityCompress(dir,"application/zip");
		}*/

	}

	public void fileOperation(String fileName,int operation){
	
		switch (operation){
		case COPIER:
			
			break;
		case COLLER:
		
			break;
		case COUPER:
		
			break;
		case SUPPRIMER:
			
			break;
		default:
			Toast.makeText(getBaseContext(),
					"DEFAULT ID " , Toast.LENGTH_SHORT)
					.show();
		}
		
		
	}
	
	
	
	
	
	
	
	
	@SuppressWarnings("unchecked")
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo){
		
		super.onCreateContextMenu(menu, v, menuInfo);
		
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) menuInfo;

		HashMap<String, String> map;
		map = (HashMap<String, String>) simpleAdapter.getItem(info.position);
		Set<String> st = map.keySet();
		Iterator<String> itr = st.iterator();
		String second = itr.next();
		second = itr.next();
		sContextMenuFileName = map.get(second);
		
		menu.setHeaderTitle(sContextMenuFileName);
		
		menu.add(Menu.NONE,COPIER,0,"Copier");	
		menu.add(Menu.NONE,COLLER,0,"Coller");		
		menu.add(Menu.NONE,COUPER,0,"Couper");		
		menu.add(Menu.NONE,SUPPRIMER,0,"Supprimer");	
		
		
		
		
	}
	
	
	public boolean onContextItemSelected(MenuItem item){
		
		//item.
		
		FileUtils fu = new FileUtils();
		String sTmp = "/sdcard/andexplorer";
		String sErreur=null;
		int iErreur=0;
		switch (item.getItemId()){
		case COUPER:
			//break;
			//resultForContextMenu.remove()
			
			simpleAdapter = new SimpleAdapter(AndExplorerActivity.this, resultForContextMenu,
					R.layout.row, new String[] { "img", "directory" },
					new int[] { R.id.img, R.id.directory });
			listViewFile.setAdapter(simpleAdapter);
	
	
		case COPIER:
			fu.copy(sContextMenuFileName, sTmp);
			Toast.makeText(getBaseContext(),
					"Copie effectue " , Toast.LENGTH_SHORT)
					.show();
			break;
		case COLLER:
			iErreur = fu.paste(sTmp, sContextMenuFileName);
			if (iErreur != 0){
				if (iErreur == -1){
					sErreur = " fichier ";
				}
				else{
					sErreur = " dossier ";
				}
				Toast.makeText(getBaseContext(),
						"Erreur "+sErreur+" existe dans la dossier destinataire" , Toast.LENGTH_SHORT)
						.show();
				break;
			}
			Toast.makeText(getBaseContext(),
					"Coller effectue " , Toast.LENGTH_SHORT)
					.show();
			break;
		case SUPPRIMER:
			fu.deleteAll(sContextMenuFileName);
			Toast.makeText(getBaseContext(),
					"Suppression effectue " , Toast.LENGTH_SHORT)
					.show();
			break;
		default:
			Toast.makeText(getBaseContext(),
					"Default ne dois pas se produire" , Toast.LENGTH_SHORT)
					.show();
		}
		return true;
	}
	
	
	
	
	
	
	
	/* Action effectuer apres le clic sur un element de type mp3,png ...*/
	
	public void FileActivity(String filedir, String type) {
		Intent intent = new Intent(android.content.Intent.ACTION_VIEW);
		File file = new File(filedir);
		intent.setDataAndType(Uri.fromFile(file), type);
		startActivity(intent);
	}
	/*
	public void FileActivityCompress(String filedir,String type){
		Intent intent = new Intent();
		File file = new File(filedir);
		intent.setDataAndType(Uri.fromFile(file),type);
		startActivity(intent);
	}*/
	
	
	/* Classs interne FileOnSystem */
	
	private class FileOnSystem extends
			AsyncTask<String, Void, ArrayList<HashMap<String, String>>> {

		/* methode execute en arriere plan*/
		
		protected ArrayList<HashMap<String, String>> doInBackground(
				String... params) {
			HashMap<String, String> map;
			ArrayList<HashMap<String, String>> AlistItem = new ArrayList<HashMap<String, String>>();
			String sType;

			File file = new File(params[0]);
			File listfiles[] = file.listFiles();
			int size = listfiles.length;
			for (int i = 0; i < size; i++) {

				map = new HashMap<String, String>();
				map.put("directory", "" + listfiles[i]);
				sType = getType(listfiles[i]);
				map.put("img", sType);
				AlistItem.add(map);
			}
			return AlistItem;
		}

		/* methode renvoyant le type de la donnee presente sur le systeme de fichier*/
		
		private String getType(File file) {
			String sFile;
			int i = 0;
			if (file.isDirectory()) {
				return sdFOL;
			}
			sFile = "" + file;
			if (sFile.endsWith(sAPK)) {
				return sdAPK;
			}
			if (sFile.endsWith(sZIP)) {
				return sdZIP;
			}
			if (sFile.endsWith(sTXT)) {
				return sdTXT;
			}
			if (sFile.endsWith(sPDF)) {
				return sdPDF;
			}

			while (i < sTypesMusique.length) {
				if (sFile.endsWith(sTypesMusique[i])) {
					return sdMUS;
				}
				i++;
			}
			i = 0;
			while (i < sTypesVideo.length) {
				if (sFile.endsWith(sTypesVideo[i])) {
					return sdVID;
				}
				i++;
			}
			i = 0;
			while (i < sTypesImage.length) {
				if (sFile.endsWith(sTypesImage[i])) {
					return sdICO;
				}
				i++;
			}
			i=0;
			while (i < sTypesRead.length){
				if (sFile.endsWith(sTypesRead[i])){
					return sdTXT;
				}
				i++;
			}
			return sdQUE;
		}

		/* methode execute a la fin du traitement en arriere plan */
		
		public void onPostExecute(ArrayList<HashMap<String, String>> result) {
			/* Affichage du noubeau adapter pour l'interface graphique */
			simpleAdapter = new SimpleAdapter(AndExplorerActivity.this, result,
					R.layout.row, new String[] { "img", "directory" },
					new int[] { R.id.img, R.id.directory });
			listViewFile.setAdapter(simpleAdapter);
			//
			resultForContextMenu = result ;
			registerForContextMenu(listViewFile);

		}

	}

}