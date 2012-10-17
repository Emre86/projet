package com.exercise.AndroidList;

import android.app.ListActivity;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class AndroidList extends ListActivity {
	
	public class MyCustomAdapter extends ArrayAdapter<String> {

		public MyCustomAdapter(Context context, int textViewResourceId,
				String[] objects) {
			super(context, textViewResourceId, objects);
			// TODO Auto-generated constructor stub
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			// TODO Auto-generated method stub
			//return super.getView(position, convertView, parent);
			LayoutInflater inflater=getLayoutInflater();
			View row=inflater.inflate(R.layout.row, parent, false);
			TextView label=(TextView)row.findViewById(R.id.weekofday);
			label.setText(DayOfWeek[position]);
			ImageView icon=(ImageView)row.findViewById(R.id.icon);
			
			if (DayOfWeek[position]=="Sunday"){
				icon.setImageResource(R.drawable.icon);
			}
			else{
				icon.setImageResource(R.drawable.icongray);
			}
			
			return row;
		}
	}

	String[] DayOfWeek = {"Sunday", "Monday", "Tuesday",
			"Wednesday", "Thursday", "Friday", "Saturday"
	};
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.main);
        /*setListAdapter(new ArrayAdapter<String>(this, 
        		R.layout.row, R.id.weekofday, DayOfWeek));*/
        setListAdapter(new MyCustomAdapter(AndroidList.this, R.layout.row, DayOfWeek));
    }

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		// TODO Auto-generated method stub
		//super.onListItemClick(l, v, position, id);
		String selection = l.getItemAtPosition(position).toString();
		Toast.makeText(this, selection, Toast.LENGTH_LONG).show();
	}
	
}