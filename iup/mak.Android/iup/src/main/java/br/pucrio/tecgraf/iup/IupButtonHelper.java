package br.pucrio.tecgraf.iup;
import androidx.appcompat.view.ContextThemeWrapper;
import androidx.appcompat.widget.AppCompatButton;
import android.view.View;
import android.widget.Button;

public final class IupButtonHelper
{
	// value must be final in order to access in inner class
	public static Button createButton(final long ihandle_ptr)
	{
		//Context context = (Context)IupApplication.getIupApplication();
		ContextThemeWrapper theme_context = IupCommon.getContextThemeWrapper();
		Button new_button = new AppCompatButton(theme_context);

		String attrib_string = IupCommon.iupAttribGet(ihandle_ptr, "TITLE");

		android.util.Log.w("IupButtonHelper::createButton", "attrib_string: " + attrib_string);

		if(null != attrib_string)
		{
			new_button.setText(attrib_string);
		}


		new_button.setOnClickListener(new View.OnClickListener()
		{
			public void onClick(View v)
			{
				IupCommon.handleIupCallback(ihandle_ptr, "ACTION");
			}
		});



		return new_button;
	}
	
}


