package com.android.andexplorer;

import java.io.File;

import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;

class FileUtils {

	public void deleteAll(String sFileDelete) {
		String sTmp = null;
		File fileDelete = new File(sFileDelete);
		if (fileDelete.exists()) {
			if (fileDelete.isFile()) {
				fileDelete.delete();
			}
			if (fileDelete.isDirectory()) {
				String sListFile[] = fileDelete.list();
				for (int i = 0; i < sListFile.length; i++) {
					sTmp = sFileDelete + "/" + sListFile[i];
					deleteAll(sTmp);
				}
				fileDelete.delete();
			}
		}
	}

	public int paste(String sTmp, String sFileDest) {
		File fTmp = new File(sTmp);

		if (fTmp.exists()) {
			File flist[] = fTmp.listFiles();
			File fileDest = new File(sFileDest);
			if (fileDest.isDirectory()) {
				if (flist.length != 0) {
					String fName = flist[0].getName();
					File flistDest[] = fileDest.listFiles();
					for (int i = 0; i < flistDest.length; i++) {
						if (fName.equals(flistDest[i].getName())) {
							if (flistDest[i].isFile()) {
								return -1;
							} else {
								return -2;
							}
						}
					}
					if (flist[0].isDirectory()) {
						copyDirectory(fTmp, fileDest);
					}
					if (flist[0].isFile()) {
						sFileDest = sFileDest + "/" + flist[0].getName();
						File fileDest2 = new File(sFileDest);
						copyFile(flist[0], fileDest2);
					}

				}
			}
		}
		return 0;
	}

	public void copy(String sFileSrc, String sTmp) {
		File fileSrc = new File(sFileSrc);

		if (fileSrc.exists()) {
			File fTmp = new File(sTmp);
			if (fTmp.exists()) {
				deleteAll(sTmp);
				fTmp.mkdirs();
			} else {
				fTmp.mkdirs();
			}
			File fileDest = new File(fTmp, fileSrc.getName());
			if (fileSrc.isFile()) {
				copyFile(fileSrc, fileDest);
			}
			if (fileSrc.isDirectory()) {
				copyDirectory(fileSrc, fileDest);
			}
		}
	}

	public void copyDirectory(File fileSrc, File fileDest) {
		if (!fileDest.exists()) {
			fileDest.mkdirs();
		}
		File[] inDir = fileSrc.listFiles();
		for (int i = 0; i < inDir.length; i++) {
			File file = inDir[i];
			if (file.isDirectory()) {
				copyDirectory(file, new File(fileDest, file.getName()));
			}
			if (file.isFile()) {
				copyFile(file, new File(fileDest, file.getName()));
			}
		}
	}

	public void copyFile(File fileSrc, File fileDest) {

		try {

			DataInputStream lecteur = new DataInputStream(
					new BufferedInputStream(new FileInputStream(fileSrc)));

			DataOutputStream ecrivain = new DataOutputStream(
					new BufferedOutputStream(new FileOutputStream(fileDest)));

			int iLength = (int) fileSrc.length();
			byte tab[] = new byte[iLength];
			lecteur.read(tab, 0, iLength);
			ecrivain.write(tab, 0, iLength);
			lecteur.close();
			ecrivain.close();

		} catch (FileNotFoundException fnfe) {
			fnfe.printStackTrace();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}

	}

}
