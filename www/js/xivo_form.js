var xivo_fm = document.forms;
var xivo_fm_error = new Array();

function xivo_fm_show_error()
{
	if(xivo_is_undef(xivo_fm_error) == true
	|| xivo_is_array(xivo_fm_error) == false
	|| xivo_is_undef(xivo_fm_error_class) == true)
		return(false);

	for(var key in xivo_fm_error)
	{
		var val = new Boolean(xivo_fm_error[key]);

		if((obj = xivo_eid(key)) == false || val == false)
			continue;

		switch(obj.tagName.toLowerCase())
		{
			case 'input':
				if(obj.type != 'text'
				&& obj.type != 'file'
				&& obj.type != 'password')
					continue;
			default:
				obj.className = xivo_fm_error_class;
		}
	}

	return(true);
}

function xivo_fm_set_onfocus(obj)
{
	var arr = new Array();
	arr['input'] = 1;
	arr['select'] = 1;
	arr['textarea'] = 1;

	if(xivo_is_undef(xivo_fm_onfocus_class) == true
	|| xivo_is_undef(xivo_fm_error_class) == true
	|| xivo_is_undef(obj.tagName) == true
	|| xivo_is_undef(arr[obj.tagName.toLowerCase()]) == true)
		return(false);
	else if((obj.tagName.toLowerCase() == 'input'
	&& obj.type != 'text'
	&& obj.type != 'file'
	&& obj.type != 'password') == true
	|| obj.className == xivo_fm_error_class
	|| obj.readOnly == true
	|| obj.disabled == true)
		return(false);

	obj.className = xivo_fm_onfocus_class;

	return(true);
}

function xivo_fm_set_onblur(obj)
{
	var arr = new Array();
	arr['input'] = 1;
	arr['select'] = 1;
	arr['textarea'] = 1;

	if(xivo_is_undef(xivo_fm_onblur_class) == true
	|| xivo_is_undef(xivo_fm_error_class) == true
	|| xivo_is_undef(obj.tagName) == true
	|| xivo_is_undef(arr[obj.tagName.toLowerCase()]) == true)
		return(false);
	else if((obj.tagName.toLowerCase() == 'input'
	&& obj.type != 'text'
	&& obj.type != 'file'
	&& obj.type != 'password') == true
	|| obj.className == xivo_fm_error_class
	|| obj.readOnly == true
	|| obj.disabled == true)
		return(false);

	obj.className = xivo_fm_onblur_class;

	return(true);
}

function xivo_fm_onfocus_onblur()
{
	var arr = new Array();
	arr[0] = 'input';
	arr[1] = 'select';
	arr[2] = 'textarea';

	if(xivo_is_undef(xivo_fm_error_class) == true)
		return(false);

	for(var i = 0;i < 3;i++)
	{
		if((tag = xivo_etag(arr[i])) == false
		|| (len = tag.length) == 0)
			continue;

		for(var j = 0;j < len;j++)
		{
			if((arr[i] == 'input'
			&& tag[j].type != 'text'
			&& tag[j].type != 'file'
			&& tag[j].type != 'password') == true)
				continue;

			if(xivo_is_undef(tag[j].onfocus) == true || tag[j].onfocus === null)
				tag[j].onfocus = function() { xivo_fm_set_onfocus(this); }

			if(xivo_is_undef(tag[j].onblur) == true || tag[j].onblur === null)
				tag[j].onblur = function() { xivo_fm_set_onblur(this); }
		}
	}

	return(true);
}

function xivo_fm_set_disable_submit_onenter(form)
{
	if((tag = xivo_etag('input',form)) == false
	|| (len = tag.length) == 0)
		return(null);

	for(var i = 0;i < len;i++)
	{
		if((tag[i].type === 'text' 
		   || tag[i].type === 'file'
		   || tag[i].type === 'password'
		   || tag[i].type === 'checkbox'
		   || tag[i].type === 'radio') === true
		&& (xivo_is_undef(tag[i].onkeypress) === true
		   || tag[i].onkeypress === null) === true)
			tag[i].onkeypress = function (event) { return(xivo_fm_disable_submit_onenter(event)); }
	}

	return(true);
}

function xivo_fm_move_selected(from,to)
{
	if((from = xivo_eid(from)) == false
	|| (to = xivo_eid(to)) == false
	|| from.type != 'select-multiple'
	|| to.type != 'select-multiple')
		return(false);

	var len = to.options.length;

	for(var i = 0;i < len;i++)
		to.options[i].selected = false;

	len = from.options.length;

	for(i = 0;i < len;i++)
	{
		if(from.options[i].selected != true)
			continue;

		to.options[to.options.length] = new Option(from.options[i].text,from.options[i].value);
		to.options[to.options.length-1].selected = true;
		from.options[i--] = null;	
		len--;
	}

	return(true);
}

function xivo_fm_copy_select(from,to)
{
	if((from = xivo_eid(from)) == false
	|| (to = xivo_eid(to)) == false
	|| (from.type != 'select-one'
	   && from.type != 'select-multiple') == true
	|| (to.type != 'select-one'
	   && to.type != 'select-multiple') == true)
		return(false);
	else if(to.selectedIndex == -1 || xivo_is_undef(to.options[to.selectedIndex]) == true)
		var selected = false;
	else
		var selected = to.options[to.selectedIndex].text;

	var len = to.options.length;

	for(var i = len; i >= 0; i--)
		to.options[i] = null;

	for(i = 0; i < from.options.length; i++)
	{
		to.options[to.options.length] = new Option(from.options[i].text,from.options[i].value);

		if(selected == from.options[i].text)
			to.options[to.options.length-1].selected = true;
	}

	return(true);
}

function xivo_fm_unshift_opt_select(from,text,value)
{
	if((from = xivo_eid(from)) == false
	|| (from.type != 'select-one'
	   && from.type != 'select-multiple') == true)
		return(false);
	else if(xivo_is_undef(text) == true)
		text = '';

	if(xivo_is_undef(value) == true)
		value = text;

	var len = from.options.length;

	var noptions = new Array();

	for(var i = 0;i < len;i++)
		noptions[i] = from.options[i];

	len = noptions.length;	

	from.options[0] = new Option(text,value);

	for(i = 0;i < len;i++)
		from.options[i+1] = noptions[i];

	return(true);
}

function xivo_fm_pop_opt_select(from)
{
	if((from = xivo_eid(from)) == false
	|| (from.type != 'select-one'
	   && from.type != 'select-multiple') == true)
		return(false);

	from.options[0] = null;

	return(true);
}

function xivo_fm_get_text_opt_select(from,value)
{
	if((from = xivo_eid(from)) === false
	|| from.type !== 'select-one'
	|| xivo_is_scalar(value) === false)
		return(false);

	var r = false;
	var sltindex = from.selectedIndex;

	from.value = value;
	var valueindex = from.selectedIndex;

	if(xivo_is_undef(from.options[valueindex]) === false)
		r = from.options[valueindex].text;

	from.selectedIndex = sltindex;

	return(r);
}

function xivo_fm_select(from,select)
{
	if((from = xivo_eid(from)) == false || from.type != 'select-multiple')
		return(false);

	select = select == false ? false : true;

	var len = from.options.length;

	for(var i = 0;i < len;i++)
		from.options[i].selected = select;

	return(true);
}

function xivo_fm_order_selected(from,order,num)
{
	order = Number(order);

	if((from = xivo_eid(from)) == false || from.type != 'select-multiple')
		return(false);

	var len = from.length;
	var selected = from.selectedIndex;

	if(len < 2 || selected == -1 || xivo_is_undef(from.options[selected]) == true)
		return(false);
	else if(order == -1)
	{
		if(selected == len-1 || xivo_is_undef(from.options[selected+1]) == true)
			return(false);

		var noption = from.options[selected+1];
		var soption = from.options[selected];

		if(Boolean(num) === false)
		{
			ntext = noption.text;
			stext = soption.text;
		}
		else
		{
			if((rs = noption.text.match(/^(\d+)\. (.*)$/)) != null)
				ntext = rs[2];
			else
				ntext = noption.text;

			ntext = (selected+1)+'. '+ntext;

			if((rs = soption.text.match(/^(\d+)\. (.*)$/)) != null)
				stext = rs[2];
			else
				stext = soption.text;

			stext = (selected+2)+'. '+stext;
		}

		from.options[selected+1] = new Option(stext,soption.value);
		from.options[selected] = new Option(ntext,noption.value);
		
		xivo_fm_select(from.id,false);

		from.options[selected+1].selected = true;
	}
	else
	{
		if(selected == 0 || xivo_is_undef(from.options[selected-1]) == true)
			return(false);

		var noption = from.options[selected-1];
		var soption = from.options[selected];

		if(Boolean(num) === false)
		{
			ntext = noption.text;
			stext = soption.text;
		}
		else
		{
			if((rs = noption.text.match(/^(\d+)\. (.*)$/)) != null)
				ntext = rs[2];
			else
				ntext = noption.text;

			ntext = (selected+1)+'. '+ntext;

			if((rs = soption.text.match(/^(\d+)\. (.*)$/)) != null)
				stext = rs[2];
			else
				stext = soption.text;

			stext = selected+'. '+stext;
		}

		from.options[selected-1] = new Option(stext,soption.value);
		from.options[selected] = new Option(ntext,noption.value);

		xivo_fm_select(from.id,false);

		from.options[selected-1].selected = true;
	}
}

function xivo_fm_select_add_entry(id,text,value,num)
{
	if ((obj = xivo_eid(id)) == false
	|| (obj.type != 'select-multiple'
	   && obj.type != 'select-one') == true)
		return(false);
	else if(xivo_is_undef(text) == true)
		text = '';

	if(xivo_is_undef(value) == true)
		value = null;

	var len = obj.options.length;

	if(Boolean(num) === true)
		text = (len + 1)+'. '+text;

	obj.options[len] = new Option(text,value);
	obj.scrollTop = obj.scrollHeight;

	return(true);
}

function xivo_fm_select_delete_entry(id,num)
{
	if ((obj = xivo_eid(id)) == false
	|| (obj.type != 'select-multiple'
	   && obj.type != 'select-one') == true)
		return(false);
	
	var len = obj.options.length;

	if(Boolean(num) === false)
	{
		for(i = len-1;i >= 0;i--)
		{
			if(obj.options[i].selected == true)
				obj.options[i] = null;
		}

		return(true);
	}

	for(i = 0;i < len;i++)
	{
		if(obj.options[i].selected == true)
		{
			obj.options[i--] = null;	
			len--;
			continue;
		}
		else if((rs = obj.options[i].text.match(/^(\d+)\. (.*)$/)) != null)
			text = rs[2];
		else
			text = obj.options[i].text;

		obj.options[i].text = (i+1)+'. '+text;
	}

	return(true);
}

function xivo_fm_unshift_pop_opt_select(from,text,value,chk,num)
{
	if((from = xivo_eid(from)) == false
	|| (from.type != 'select-one'
	   && from.type != 'select-multiple') == true)
		return(false);
	else if(xivo_is_undef(text) == true)
		text = '';

	if(xivo_is_undef(value) == true)
		value = null;

	if(xivo_is_undef(chk) == true)
		chk = '';

	if(xivo_is_undef(num) == true)
		num = 0;

	num = Number(num);

	if(from.value == chk)
		xivo_fm_unshift_opt_select(from.id,text,value);

	if(from.value == chk && xivo_is_undef(from.options[num]) == false && from.options[num].value == 'null')
		xivo_fm_pop_opt_select(from.id);

	return(true);
}

function xivo_fm_field_disabled(obj,disable)
{
	if(xivo_is_object(obj) == false)
		return(false);
	else if(xivo_is_undef(disable) == true)
		disable = false;

	disable = Boolean(disable);

	if((tag_input = xivo_etag('input',obj)) != false)
	{
		var tag_input_nb = tag_input.length;

		for(i = 0;i < tag_input_nb;i++)
			tag_input[i].disabled = disable;
	}

	if((tag_select = xivo_etag('select',obj)) != false)
	{
		var tag_select_nb = tag_select.length;

		for(i = 0;i < tag_select_nb;i++)
			tag_select[i].disabled = disable;
	}

	if((tag_textarea = xivo_etag('textarea',obj)) != false)
	{
		var tag_textarea_nb = tag_textarea.length;

		for(i = 0;i < tag_textarea_nb;i++)
			tag_textarea[i].disabled = disable;
	}

	return(true);
}

function xivo_fm_field_id_counter(obj,cnt)
{
	if(xivo_is_object(obj) == false || xivo_is_int(cnt) == false)
		return(false);

	var taglist = ['input','select','textarea','a','span'];
	var len = taglist.length;

	for(var i = 0;i < len;i++)
	{
		if((tagobj = xivo_etag(taglist[i],obj)) === false)
			continue;

		tagobj_nb = tagobj.length;

		for(j = 0;j < tagobj_nb;j++)
		{
			if(xivo_is_undef(tagobj[j].id) == false
			&& tagobj[j].id.length > 0)
				tagobj[j].id += '-'+cnt;
		}
	}

	return(true);
}

function xivo_fm_field_name_counter(obj,cnt)
{
	if(xivo_is_object(obj) == false || xivo_is_int(cnt) == false)
		return(false);
	else if((tag_input = xivo_etag('input',obj)) != false)
	{
		var tag_input_nb = tag_input.length;

		for(i = 0;i < tag_input_nb;i++)
		{
			if(xivo_is_undef(tag_input[i].name) == false)
				tag_input[i].name += '['+cnt+']';
		}
	}

	if((tag_select = xivo_etag('select',obj)) != false)
	{
		var tag_select_nb = tag_select.length;

		for(i = 0;i < tag_select_nb;i++)
		{
			if(xivo_is_undef(tag_select[i].name) == false)
				tag_select[i].name += '['+cnt+']';
		}
	}

	if((tag_textarea = xivo_etag('textarea',obj)) != false)
	{
		var tag_textarea_nb = tag_textarea.length;

		for(i = 0;i < tag_textarea_nb;i++)
		{
			if(xivo_is_undef(tag_textarea[i].name) == false)
				tag_textarea[i].name += '['+cnt+']';
		}
	}

	return(true);
}

function xivo_fm_mk_acl(tree)
{
        var ref = tree.form[tree.name];
        var value = tree.value;
        var len = value.length;
        var nb = ref.length
        var sub = 0;
        var rs = false;

        for(i = 0;i < nb;i++)
        {
                sub = ref[i].value.substring(0,len);
                rs = tree.checked == true ? true : false;

                if(value == sub)
                        ref[i].checked = rs;
        }
}

function xivo_fm_readonly(list,enable)
{
	if(xivo_is_array(list) == false)
		list = new Array(list);

	enable = new Boolean(enable);

	nb = list.length;

	for(var i = 0;i < nb;i++)
	{
		if((element = xivo_eid(list[i])) == false)
			continue;
		else if(enable == true)
		{
			element.disabled = false;
			element.readOnly = false;
			element.className = xivo_fm_enabled_class;
		}
		else
		{
			if(element.tagName.toLowerCase() == 'select')
				element.disabled = true;
			else
				element.readOnly = true;
			element.className = xivo_fm_readonly_class;
		}
	}
}

function xivo_fm_select_add_host_ipv4_subnet(id,value)
{
	if(xivo_chk_ipv4_strict(value) == false
	&& xivo_chk_host(value) == false
	&& xivo_chk_ipv4_subnet(value) == false)
		return(false);

	return(xivo_fm_select_add_entry(id,value,value));
}

function xivo_fm_checked_all(form,name,mode)
{
	if(xivo_is_undef(form) == true
	|| xivo_is_undef(name) == true
	|| xivo_is_string(form) == false
	|| xivo_is_string(name) == false
	|| xivo_is_undef(xivo_fm[form]) == true
	|| xivo_is_undef(xivo_fm[form][name]) == true)
		return(false);
	else if(xivo_is_undef(mode) == true)
		mode = true;
	else if(mode != 'reverse')
		mode = Boolean(mode);

	ref = xivo_fm[form][name];

	if(xivo_is_undef(ref.length) == false)
		len = ref.length;
	else if(xivo_is_undef(ref.type) == true
	|| (ref.type != 'checkbox' && ref.type != 'radio') == true)
		return(false);
	else
	{
		if(mode != 'reverse')
			ref.checked = mode;
		else if(ref.checked == true)
			ref.checked = false;
		else
			ref.checked = true;

		return(true);
	}

	for(var i = 0;i < len;i++)
	{
		if(ref[i].type != 'checkbox' && ref[i].type != 'radio')
			continue;

		if(mode != 'reverse')
			ref[i].checked = mode;
		else if(ref[i].checked == true)
			ref[i].checked = false;
		else
			ref[i].checked = true;
	}

	return(true);
}

function xivo_fm_get_checked(form,name)
{
	if(xivo_is_undef(form) == true
	|| xivo_is_undef(name) == true
	|| xivo_is_string(form) == false
	|| xivo_is_string(name) == false
	|| xivo_is_undef(xivo_fm[form]) == true
	|| xivo_is_undef(xivo_fm[form][name]) == true)
		return(false);

	ref = xivo_fm[form][name];

	if(xivo_is_undef(ref.length) == false)
		len = ref.length;
	else if(xivo_is_undef(ref.type) == true
	|| (ref.type != 'checkbox' && ref.type != 'radio') == true)
		return(false);
	else
		return(0);

	for(var i = 0;i < len;i++)
	{
		if((ref[i].type == 'checkbox' || ref[i].type == 'radio') == true
		&& ref[i].checked == true)
			return(i);
	}

	return(false);
}

function xivo_fm_get_value_from_key(form,name,key)
{
	if(xivo_is_undef(form) == true
	|| xivo_is_undef(name) == true
	|| xivo_is_string(form) == false
	|| xivo_is_string(name) == false
	|| xivo_is_undef(xivo_fm[form]) == true
	|| xivo_is_undef(xivo_fm[form][name]) == true
	|| xivo_is_undef(xivo_fm[form][name][key]) == true
	|| xivo_is_undef(xivo_fm[form][name][key].value) == true)
		return(false);

	return(xivo_fm[form][name][key].value);
}

function xivo_fm_enable_disable_field(form,name,disable,exform,exformtag)
{
	if(xivo_is_undef(form) == true
	|| xivo_is_undef(name) == true
	|| xivo_is_object(form) == false
	|| xivo_is_string(name) == false
	|| xivo_is_undef(form[name]) == true)
		return(false);

	if(xivo_is_string(exform) == true && xivo_is_string(exformtag) == true)
		var disableparent = false;
	else
		var disableparent = true;

	if((disable = Boolean(disable) != false))
		var classname = xivo_fm_disabled_class;
	else
		var classname = xivo_fm_enabled_class;

	var ref = form[name];

	if(xivo_is_undef(ref.tagName) == true)
	{
		if(xivo_is_undef(ref.item) == true
		|| xivo_is_undef(ref.length) == true)
			return(false);
		
		var nb = ref.length;

		for(var i = 0;i < nb;i++)
		{
			if(xivo_is_undef(ref[i]) == false)
			{
				if(disableparent == false
				&& xivo_get_parent_by_tagname(ref[i],exformtag).id == exform)
					continue;

				ref[i].disabled = disable;
				ref[i].className = classname;
			}
		}

		return(true);
	}

	switch(ref.tagName.toLowerCase())
	{
		case 'input':
		case 'select':
		case 'textarea':
			if(disableparent == false
			&& xivo_get_parent_by_tagname(ref,exformtag).id == exform)
				return(false);

			ref.disabled = disable;
			ref.className = classname;
		default:
			return(false);
	}

	return(true);
}

function xivo_fm_reset_field(obj,selected)
{
	if(xivo_is_undef(obj.tagName) == true
	|| xivo_is_undef(obj.type) == true)
		return(false);

	selected = selected === true;

	switch(obj.tagName.toLowerCase())
	{
		case 'input':
			if(obj.type == 'checkbox'
			|| obj.type == 'radio')
				obj.checked = obj.defaultChecked;
			else if(obj.type == 'text'
			|| obj.type == 'file'
			|| obj.type == 'password')
				obj.value = obj.defaultValue;
			break;
		case 'textearea':
				obj.value = obj.defaultValue;
			break;
		case 'select':
			if(obj.type != 'select-multiple' && obj.type != 'select-one')
				return(false);

			var len = obj.options.length;

			for(i = 0; i < len; i++)
				obj.options[i].selected = obj.options[i].defaultSelected;
			break;
	}

	return(true);
}

function xivo_fm_reset_child_field(obj)
{
	if(xivo_is_object(obj) == false)
		return(false);

	if((tag_input = xivo_etag('input',obj)) != false)
	{
		var tag_input_nb = tag_input.length;

		for(i = 0;i < tag_input_nb;i++)
			xivo_fm_reset_field(tag_input[i]);
	}

	if((tag_select = xivo_etag('select',obj)) != false)
	{
		var tag_select_nb = tag_select.length;

		for(i = 0;i < tag_select_nb;i++)
			xivo_fm_reset_field(tag_select[i]);
	}

	if((tag_textarea = xivo_etag('textarea',obj)) != false)
	{
		var tag_textarea_nb = tag_textarea.length;

		for(i = 0;i < tag_textarea_nb;i++)
			xivo_fm_reset_field(tag_textarea[i]);
	}

	return(true);
}

function xivo_fm_disable_submit_onenter(e)
{
	return(((window.event) ? event.keyCode : e.which) != 13);
}
