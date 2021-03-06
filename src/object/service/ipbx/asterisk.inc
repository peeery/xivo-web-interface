<?php

#
# XiVO Web-Interface
# Copyright 2006-2016 The Wazo Authors  (see the AUTHORS file)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

require_once(dwho_file::joinpath(XIVO_PATH_OBJECT,'service','abstract.inc'));

class xivo_service_asterisk extends xivo_service_abstract
{
	var $_ini                = array();
	var $_type               = 'ipbx';
	var $_name               = 'asterisk';
	var $_identity           = array();
	var $_realstatic         = array();
	var $_apprealstatic      = array();
	var $_request_handlers   = null;
	var $_sysconfd           = null;

	public function __construct($ini)
	{
		$this->_load_config();

		if(dwho_issa('general',$ini) === false)
			trigger_error('Invalid parameter in '.__CLASS__,E_USER_ERROR);

		$this->_ini = $ini;

		$this->_load_service_information();
	}

	public function datastorage_general_config()
	{
		if(dwho_issa('datastorage',$this->_ini['general']) === false)
			return(false);

		return($this->_ini['general']['datastorage']);
	}

	public function required($arr,$once=false,$includable=false)
	{
		$arr = (array) $arr;

		array_unshift($arr,dirname(__FILE__).DWHO_SEP_DIR.'asterisk');

		return(dwho_file::required($arr,$once,$includable));
	}

	public function discuss_ipbx($cmd)
	{
		if (! is_object($this->_request_handlers)) {
			$_XOBJ = dwho_gct::get('xivo_object');
			if ($_XOBJ) {
				$this->_request_handlers = &$_XOBJ->get_module('request_handlers');
				if(! $this->_request_handlers) {
					return(($this->_request_handlers = false));
				}
			} else {
				return(($this->_request_handlers = false));
			}
		}

		$commands = array();
		$commands['ipbx'] = array($cmd);

		return($this->_request_handlers->exec_cmd($commands));
	}

	private function _load_sysconfd()
	{
		if (! is_object($this->_sysconfd)) {
			$_XOBJ = dwho_gct::get('xivo_object');
			if ($_XOBJ) {
				$this->_sysconfd = &$_XOBJ->get_module('sysconfd');
				if(! $this->_sysconfd) {
					return(($this->_sysconfd = false));
				}
			} else {
				return(false);
			}
		}

		return(true);
	}

	public function core_show_version()
	{
		if($this->_load_sysconfd() === false
		|| ($info = $this->_sysconfd->request_get('/core_show_version')) === false
		|| ($this->_sysconfd->last_status_code() !== 200))
			return(false);

		return($info);
	}

	public function core_show_channels()
	{
		if($this->_load_sysconfd() === false
		|| ($info = $this->_sysconfd->request_get('/core_show_channels')) === false
		|| ($this->_sysconfd->last_status_code() !== 200))
			return(false);

		return($info);
	}

	public function sip_show_peer($peer)
	{
		$query = array(
			array('peer', $peer)
		);

		if($this->_load_sysconfd() === false
		|| ($info = $this->_sysconfd->request_get('/sip_show_peer', $query)) === false
		|| ($this->_sysconfd->last_status_code() !== 200))
			return(false);

		return($info);
	}

	private function _load_identity_class($module)
	{
		$dir = dwho_file::joinpath(XIVO_PATH_OBJECT,
					   'service',
					   $this->_type,
					   $this->_name,
					   $module);

		$class = 'xivo_service_'.$this->_name.'_'.$module.'_identity';

		if(dwho_file::load_file('identity.inc',$dir,true) === false
		|| class_exists($class) === false)
			return(false);

		return($class);
	}

	private function _load_identity($module)
	{
		if($this->chk_module_enable($module) === false)
		{
			trigger_error('Invalid module in '.$module.' '.$this->_name.' to load identity class',E_USER_ERROR);
			return(false);
		}
		else if(isset($this->_identity[$module]) === true)
			return(true);
		else if(($class = $this->_load_identity_class($module)) === false)
		{
			trigger_error($module.' identity class does not exist in '.$this->_name,E_USER_ERROR);
			return(false);
		}
		else
			$this->_identity[$module] = new $class($this);

		return(true);
	}

	public function &get_identity($module)
	{
		$r = false;

		if($this->_load_identity($module) === true)
			$r = &$this->_identity[$module];

		return $r;
	}

	public function identity($module,$arr,$full=true)
	{
		if(($identity = &$this->get_identity($module)) === false)
			return(false);

		return($identity->mk_identity($arr,$full));
	}

	public function load_realstatic_class($module)
	{
		if($this->chk_realstatic_enable($module) === false)
		{
			trigger_error('Invalid realtime static '.$module.' in '.$this->_name,E_USER_ERROR);
			return(false);
		}

		return($this->_load_realstatic_class($module));
	}

	private function _load_realstatic_class($name)
	{
		$file = $name.'.inc';

		$dir = dwho_file::joinpath(XIVO_PATH_OBJECT,
					   'service',
					   $this->_type,
					   $this->_name);

		$class = 'xivo_service_'.$this->_name.'_realstatic';

		$dirapi = $dir.DWHO_SEP_DIR.$name;
		$classapi = $class.'_'.$name;

		if(dwho_file::load_file($file,$dirapi,true) !== false && class_exists($classapi) === true)
			return($classapi);

		$file = 'realstatic.inc';
		$dir .= DWHO_SEP_DIR.'realstatic';

		if(dwho_file::load_file($file,$dir,true) !== false && class_exists($class) === true)
			return($class);

		return(false);
	}

	public function chk_realstatic_enable($name)
	{
		return($this->_chk_exists_cfg('realstatic',$name));
	}

	private function _load_realstatic($name,$param=null)
	{
		if($this->chk_realstatic_enable($name) === false)
		{
			trigger_error('Invalid realtime static '.$name.' in '.$this->_name,E_USER_ERROR);
			return(false);
		}

		$ref = &$this->_conf['realstatic'][$name];

		if(isset($this->_realstatic[$name]) === true)
		{
			if(is_array($param) === true)
				$this->_realstatic[$name]->load_init($param);

			return(true);
		}
		else if(($class = $this->_load_realstatic_class($name)) === false)
		{
			trigger_error('Realtime Static '.$name.' does not exist in '.$this->_name,E_USER_ERROR);
			return(false);
		}
		else if(isset($ref['datastorage']) === false || (bool) $ref['datastorage'] === false)
			$this->_realstatic[$name] = new $class($this,$param);
		else if(is_string($ref['datastorage']) === true && $this->chk_realstatic_enable($ref['datastorage']) === true)
			$this->_realstatic[$name] = new $class($this,$this->_load_dso($ref['datastorage']),$param);
		else
			$this->_realstatic[$name] = new $class($this,$this->_load_dso($name),$param);

		return(true);
	}

	public function &get_realstatic($name,$param=null)
	{
		$r = false;

		if($this->_load_realstatic($name,$param) === true)
			$r = &$this->_realstatic[$name];

		return $r;
	}

	private function _load_apprealstatic_class($application)
	{
		$application = (string) $application;

		$file = $application.'.inc';
		$dir = dwho_file::joinpath(XIVO_PATH_OBJECT,
					   'application',
					   'service',
					   $this->_type,
					   $this->_name,
					   'realstatic',
					   $application);

		$class = 'xivo_application_service_'.$this->_name.'_realstatic_'.$application;

		if(dwho_file::load_file($file,$dir,true) !== false && class_exists($class) === true)
			return($class);

		return(false);
	}

	public function &call_apprealstatic($name,$func,$args=null)
	{
		$name = (string) $name;
		$func = (string) $func;

		if($this->_load_apprealstatic($name) === false)
			$r = false;
		else if($args === null)
			$r = call_user_func(array($this->_apprealstatic[$name],$func));
		else
		{
			$args = (array) $args;
			$r = call_user_func_array(array($this->_apprealstatic[$name],$func),$args);
		}

		return $r;
	}

	public function &get_apprealstatic($name)
	{
		$r = false;

		if($this->_load_apprealstatic($name) === true)
			$r = &$this->_apprealstatic[$name];

		return $r;
	}

	private function _load_apprealstatic($name)
	{
		$name = (string) $name;

		if(isset($this->_apprealstatic[$name]) === true)
			return(true);
		else if(($class = $this->_load_apprealstatic_class($name)) === false)
		{
			trigger_error('RealStatic Application '.$name.' does not exist in '.$this->_name,E_USER_ERROR);
			return(false);
		}

		$this->_apprealstatic[$name] = new $class($this);

		return(true);
	}

	private function _load_service_information()
	{
		$uppertype	= strtoupper($this->_type);
		$define_label	= 'XIVO_SRE_'.$uppertype.'_LABEL';
		$define_version	= 'XIVO_SRE_'.$uppertype.'_VERSION';

		if(defined($define_label) === true || defined($define_version) === true)
		{
			if(defined($define_label) === false)
				define($define_label,'unknown');

			if(defined($define_version) === false)
				define($define_version,'unknown');

			return(false);
		}

		if(isset($_SESSION['_SRE'][$this->_type][$this->_name]) === false)
		{
			define($define_label,'unknown');
			define($define_version,'unknown');
			return(false);
		}

		$sess_sre = &$_SESSION['_SRE'][$this->_type][$this->_name];

		$label = $version = false;

		if(dwho_issa('info',$sess_sre) === true
		&& (isset($sess_sre['info']['label'], $sess_sre['info']['version']) === true)
		&& ($sess_sre['info']['label'] !== 'No' && $sess_sre['info']['label'] !== 'such'))
		{
			$label		= $sess_sre['info']['label'];
			$version	= $sess_sre['info']['version'];
		}
		else if(($nversion = $this->_get_service_info_from_discuss()) !== false)
		{
			if(isset($nversion['label']) === true)
				$label = $sess_sre['info']['label'] = $nversion['label'];

			if(isset($nversion['version']) === true)
				$version = $sess_sre['info']['version'] = $nversion['version'];
		}

		if(dwho_issa('ini',$sess_sre) === true
		&& dwho_issa('general',$sess_sre['ini']) === true)
		{
			if($label === false
			&& isset($sess_sre['ini']['general']['label']) === true)
				$label = $sess_sre['ini']['general']['label'];

			if($version === false
			&& isset($sess_sre['ini']['general']['version']) === true)
				$version = $sess_sre['ini']['general']['version'];
		}

		if($label === false)
			$label = 'unknown';

		if($version === false)
			$version = 'unknown';

		define($define_label,$label);
		define($define_version,$version);
	}

	private function _get_service_info_from_discuss()
	{
		if((defined('DWHO_SESS_ENABLE') === true
			&& (bool) DWHO_SESS_ENABLE === false) === true
		|| ($info = $this->core_show_version()) === false)
			return(false);

		// Extract server name and version
		if(preg_match('/(Asterisk) (\d+(?:\.\d+)+)/',$info,$info))
		{
			return array(
				'label' => $info[1],
				'version' => $info[2]
			);
		}

		return(false);
	}

	public function get_protocol()
	{
		return($this->_conf['protocol']);
	}

	public function chk_protocol($name)
	{
		return($this->_chk_exists_cfg('protocol',strval($name)));
	}

	public function get_protocol_module_name($name)
	{
		$r = false;
		if($this->chk_protocol($name) === true)
			$r = dwho_ak('module',$this->_conf['protocol'][$name],true);

		return($r);
	}

	public function &get_protocol_module($name)
	{
		$r = false;

		if(($module = $this->get_protocol_module_name($name)) !== false)
			$r = &$this->get_module($module);

		return $r;
	}

	public function get_element($modules=array())
	{
		$modules = (array) $modules;

		if(isset($modules[0]) === false)
			$modules = array_keys($this->_conf['module']);

		if(empty($modules) === true)
			return(false);

		$r = array();

		foreach($modules as $v)
		{
			if(($module = &$this->get_module($v)) !== false
			&& ($list = $module->get_element()) !== false)
				$r[$v] = &$list;
		}

		if(empty($r) === true)
			$r = false;

		return($r);
	}

	public function get_application_element($appname)
	{
		if(($app = &$this->get_application($appname)) === false
		|| ($list = $app->get_element()) === false)
			return false;

		return(array($list));
	}

	public function get_protocol_element($protocols=array(),$module=false)
	{
		$protocols = (array) $protocols;
		$module = (bool) $module;

		if(isset($protocols[0]) === false)
			$protocols = $this->_conf['protocol'];

		if(empty($protocols) === true)
			return(false);

		$r = array();

		foreach($protocols as $k => $v)
		{
			if(isset($v['module']))
				$element = $this->get_element($v['module']);
			else if(isset($v['application']))
				$element = $this->get_application_element($v['application']);

			if($element === false)
				continue;

#			if(isset($v['module']) === false
#			|| ($element = $this->get_element($v['module'])) === false)
#				continue;
			if($module === true)
				$r[$k] = $element;
			else if(is_array($element) === true)
			{
				foreach($element as $value)
					$r[$k] = $value;
			}
		}

		if(empty($r) === true)
			$r = false;

		return($r);
	}

	public function get_formats()
	{
		return($this->_conf['format']);
	}

	public function chk_channel($channel)
	{
		return($this->_chk_exists_cfg('channel',
					      strtolower(strval($channel))));
	}

	public function chk_channel_by_protocol($protocol)
	{
		$r = false;

		$protocol = strtolower(strval($protocol));

		if($this->chk_protocol($protocol) === true
		&& $this->chk_channel($protocol) === true)
			$r = true;

		return($r);
	}

	public function get_channel_by_protocol($protocol)
	{
		$protocol = strtolower(strval($protocol));

		if($this->chk_channel_by_protocol($protocol) === true)
			return($this->_conf['channel'][$protocol]);

		return(false);
	}

	public function chk_channel_by_interface($interface)
	{
		$interface = strtolower(strval($interface));

		if(($pos = strpos($interface,'/')) !== false)
			$interface = substr($interface,0,$pos);

		if(dwho_issa('channel',$this->_conf) === false
		|| in_array($interface,$this->_conf['channel']) === false)
			return(false);

		return($interface);
	}

	public function get_channel($channel)
	{
		$r = false;

		$channel = strtolower(strval($channel));

		if($this->chk_channel($channel) === true)
			$r = $this->_conf['channel'][$channel];

		return($r);
	}

	public function mk_agent_interface($number)
	{
		$number = strval($number);

		if(ctype_digit($number) === false)
			return(false);

		return($this->mk_interface($number,null,null,null,XIVO_SRE_IPBX_AST_CHAN_AGENT));
	}

	public function mk_interface($name,$protocol='',$number='',$context='',$channel='')
	{
		$r = false;
		$name     = strtolower(strval($name));
		$protocol = strval($protocol);
		$number   = strval($number);
		$context  = strtolower(strval($context));
		$channel  = strval($channel);

		if($protocol === XIVO_SRE_IPBX_AST_PROTO_CUSTOM)
			return($name);
		else if(dwho_has_len($number) === true)
		{
			if(($channel = $this->get_channel(XIVO_SRE_IPBX_AST_CHAN_LOCAL)) !== false)
			{
				if(dwho_has_len($context) === false)
					$context = 'default';

				$r = $channel.'/'.$number.'@'.$context;
			}

			return($r);
		}

		$chan = $channel;

		if(dwho_has_len($channel) === true)
		{
			if(($channel = $this->chk_channel_by_interface($chan)) === false
			&& ($channel = $this->get_channel($chan)) === false)
				return($r);
		}
		else if(($channel = $this->get_channel_by_protocol($protocol)) === false)
			return($r);

		return($channel.'/'.$name);
	}

	public function display_extension($str)
	{
		if(isset($str{0}) === true && $str{0} === '_')
			$str = substr($str,1);

		return($str);
	}

	public function clean_extension($str)
	{
		$str = preg_replace('/\[(\d)\]/','\\1',strval($str));

		if(isset($str{0}) === true && $str{0} === '_')
		{
			$substr = substr($str,1);

			if(preg_match('/[NXZ\-\[\]\!\.]/',$substr) === 0)
				$str = $substr;

			return($str);
		}
		else if(preg_match('/[NXZ\-\[\]\!\.]/',$str) !== 0)
			$str = '_'.$str;

		return($str);
	}

	public function chk_extension($str)
	{
		$str = strval($str);

		if(isset($str{0}) === false)
			return(false);
		else if(preg_match('/[\[\-\]]/',$str) === 0)
			return($this->realexten_pattern($str));

		$split = preg_split('/\[(\d+(?:\-\d+)?)\]/',$str,-1,PREG_SPLIT_DELIM_CAPTURE);

		if(($nb = count($split)) === 0)
			return($this->realexten_pattern($str));

		$r = '';

		for($i = 0;$i < $nb;$i+=2)
		{
			if(preg_match('/[\[\-\]]/',$split[$i]) === 1)
				return(false);
			else
				$r .= $this->realexten_pattern($split[$i]);

			if(isset($split[$i + 1]) === false)
				return($r);

			$nsplit = $split[$i + 1];

			$arr = array();

			if(isset($nsplit{1}) === false)
			{
				$r .= $nsplit{0};
				continue;
			}

			if(preg_match('/^(\d*)(\d-\d)(\d*)$/',$nsplit,$match) === 1)
			{
				if($match[2]{0} >= $match[2]{2})
					return(false);

				$arr = array_flip(range($match[2]{0},$match[2]{2}));

				if(($len = strlen($match[1])) !== 0)
				{
					for($j = 0;$j < $len;$j++)
					{
						$num = $match[1]{$j};

						if(isset($arr[$num]) === true)
							return(false);

						$arr[$num] = 1;
					}
				}

				if(($len = strlen($match[3])) !== 0)
				{
					for($j = 0;$j < $len;$j++)
					{
						$num = $match[3]{$j};

						if(isset($arr[$num]) === true)
							return(false);

						$arr[$num] = 1;
					}
				}
			}
			else
			{
				$len = strlen($nsplit);

				for($j = 0;$j < $len;$j++)
				{
					$num = $nsplit{$j};

					if(isset($arr[$num]) === true)
						return(false);

					$arr[$num] = 1;
				}
			}

			ksort($arr);

			$r .= '['.implode('',array_keys($arr)).']';
		}

		return($r);
	}

	public function realexten_pattern($str)
	{
		$str = preg_replace('/\[(\d)\]/','\\1',$str);
		$pat = array('_','X','Z','N');
		$rep = array('','[0123456789]','[123456879]','[23456789]');

		return(str_replace($pat,$rep,$str));
	}

	public function exten2regexp($str)
	{
		$pat = array('_','X','Z','N','\.','\!');
		$rep = array('','[0-9]','[1-9]','[2-9]','[0-9#\*]+','[0-9#\*]*');

		return(str_replace($pat,$rep,preg_quote($str)));
	}

	public function valid_exten_pattern_in_interval($min,$pattern,$max)
	{
		if(dwho_digit($min) === false
		|| dwho_digit($max) === false)
			return(false);

		$split = preg_split('/((?:\[[0-9#\*\-\\\]+\][\+\*]?)+|[0-9])/',
				    $pattern,
				    -1,
				    (PREG_SPLIT_NO_EMPTY|PREG_SPLIT_DELIM_CAPTURE));

		if(($nb = count($split)) === 0)
			return(false);

		$curnum = &$min;
		$usemax = false;

		for($i = 0;$i < $nb;$i++)
		{
			switch($split[$i]{0})
			{
				case '[':
					if(preg_match('/^'.$split[$i].'/',$curnum,$match) !== 1)
					{
						if($usemax === true)
							return(false);

						$usemax = true;
						$curnum = &$max;

						if(preg_match('/^'.$split[$i].'/',$curnum,$match) !== 1)
							return(false);
					}

					$len = strlen($match[0]);
					$min = substr($min,$len);
					$max = substr($max,$len);
					continue;
				case '\\':
					if(substr($curnum,0,2) !== '\*')
					{
						if($usemax === true)
							return(false);

						$usemax = true;
						$curnum = &$max;

						if(substr($curnum,0,2) !== '\*')
							return(false);
					}

					$min = substr($min,2);
					$max = substr($max,2);
					continue;
				default:
					if(ctype_digit($split[$i]) === false)
						return(false);
					else if($min{0} < $split[$i]
					&& $split[$i] < $max{0})
						return(true);
					else if($min{0} > $split[$i]
					|| $max{0} < $split[$i])
						return(false);

					$min = substr($min,1);
					$max = substr($max,1);
			}
		}

		return(($min === false && $max === false));
	}

	public function valid_exten_number_in_interval($min,$number,$max)
	{
		if(dwho_has_len($max) === false)
			$max = $min;

		if(dwho_is_max_digit($min,$number,true) !== true
		|| dwho_is_max_digit($number,$max,true) !== true)
			return(false);

		return(true);
	}

	public function valid_exten_in_interval($min,$number,$max)
	{
		$min		= strval($min);
		$number		= strval($number);
		$max		= strval($max);
		$pattern	= xivo_service_asterisk::exten2regexp($number);

		if(preg_match('/^'.$pattern.'$/',$min) === 1)
			$number = $min;
		else if(preg_match('/^'.$pattern.'$/',$max) === 1)
			$number = $max;
		else if(dwho_digit($number) === false)
			return(xivo_service_asterisk::valid_exten_pattern_in_interval(
						$min,
						$pattern,
						$max));

		return(xivo_service_asterisk::valid_exten_number_in_interval($min,$number,$max));
	}

	public function mk_callerid($calleridname,$calleridnum='', $protocol=XIVO_SRE_IPBX_AST_PROTO_SIP)
	{
		$calleridnum = strval($calleridnum);
		// if calleridnum is already set by the user (calleridname looks like "name <num>")
		// we don't replace it by calleridnum
		if(preg_match('/<([^>]+)>/', $calleridname, $matches) > 0)
			$calleridnum = $matches[1];

		$calleridname = trim(preg_replace('/<[^<]*>$/','',trim(strval($calleridname))));

		if(isset($calleridname{0}) === true)
		{
			if($calleridname{0} === '"'
			&& substr($calleridname,-1) === '"')
				$calleridname = substr($calleridname,1,-1);

			$calleridname = '"'.str_replace('"','\"',str_replace('\"','"',$calleridname)).'"';
		}

		if(preg_match('/^[0-9\*#]{1,40}$/',$calleridnum) === 1)
			$calleridname .= ' <'.$calleridnum.'>';

		return(trim($calleridname));
	}

	public function parse_callerid($str)
	{
		if (preg_match('/^'.XIVO_SRE_IPBX_AST_REGEXP_CALLERID.'$/u', $str, $matches) === 1) {
			$res = array();
			if (!empty($matches[1])) {
				$res['caller_id_name'] = $matches[1];
			} else {
				$res['caller_id_name'] = $matches[2];
			}

			if (!empty($matches[3])) {
				$res['caller_id_num'] = $matches[3];
			} else {
				$res['caller_id_num'] = null;
			}
		} else {
			$res = null;
		}

		return $res;
	}

	public function get_contexts_list($arr=null)
	{
		$r = array(XIVO_SRE_IPBX_AST_CONTEXT_FEATURES,
			   XIVO_SRE_IPBX_AST_CONTEXT_GENERAL,
			   XIVO_SRE_IPBX_AST_CONTEXT_GLOBAL,
			   XIVO_SRE_IPBX_AST_CONTEXT_GLOBALS,
			   XIVO_SRE_IPBX_AST_CONTEXT_PARKEDCALLS);

		if(is_array($arr) === true)
			$r = array_merge($arr,$r);

		return($r);
	}

	public function get_codecs_list()
	{
		return(array(XIVO_SRE_IPBX_AST_CODEC_AUDIO_G723,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_GSM,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_ULAW,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_ALAW,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_ADPCM,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_SLIN,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_LPC10,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_G729,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_OPUS,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_SPEEX,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_SPEEX16,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_SPEEX32,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_ILBC,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_G726,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_G722,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_SIREN7,
			     XIVO_SRE_IPBX_AST_CODEC_AUDIO_SIREN14,
			     XIVO_SRE_IPBX_AST_CODEC_IMAGE_JPEG,
			     XIVO_SRE_IPBX_AST_CODEC_IMAGE_PNG,
			     XIVO_SRE_IPBX_AST_CODEC_TEXT_T140_RED,
			     XIVO_SRE_IPBX_AST_CODEC_TEXT_T140,
			     XIVO_SRE_IPBX_AST_CODEC_VIDEO_H261,
			     XIVO_SRE_IPBX_AST_CODEC_VIDEO_H263,
			     XIVO_SRE_IPBX_AST_CODEC_VIDEO_H263P,
			     XIVO_SRE_IPBX_AST_CODEC_VIDEO_H264));
	}

	public function get_sccp_codecs_list()
	{
		return(array(XIVO_SRE_IPBX_AST_CODEC_AUDIO_ULAW,
				XIVO_SRE_IPBX_AST_CODEC_AUDIO_ALAW,
				XIVO_SRE_IPBX_AST_CODEC_AUDIO_G722,
				XIVO_SRE_IPBX_AST_CODEC_AUDIO_G729));
	}

	public function get_amaflags_list($onlymeta=true)
	{
		if((bool) $onlymeta === true)
			return(array(XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_DEFAULT,
				     XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_OMIT,
				     XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_BILLING,
				     XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_DOCUMENTATION));

		return(array(
			XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_DEFAULT		=> XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_DEFAULT,
			XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_OMIT		=> XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_OMIT,
			XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_BILLING		=> XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_BILLING,
			XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_DOCUMENTATION	=> XIVO_SRE_IPBX_AST_CDR_AMAFLAGS_META_DOCUMENTATION));
	}

	public function chk_context($context,$arr=null)
	{
		$_reserved_context = xivo_service_asterisk::get_contexts_list($arr);
		if(is_scalar($context) === false
		|| in_array($context,$_reserved_context) === true
		|| preg_match('/^'.XIVO_SRE_IPBX_AST_REGEXP_CONTEXT.'$/',$context) !== 1)
		{
			dwho_report::push('error', dwho_i18n::babelfish('invalid_name_context',$context));
			dwho_report::push('info', dwho_i18n::babelfish('reserved_context',implode(', ',$_reserved_context)));
			return(false);
		}

		return($context);
	}

	public function get_extens_for_context_and_object($context, $type)
	{
		$where = array('context' => $context);

		switch ($type)
		{
			case 'user':
				$mod = &$this->get_module('linefeatures');
				return($this->_get_valid_extens_of_object($mod, $where));
				break;
			case 'group':
				$mod = &$this->get_module('groupfeatures');
				return($this->_get_valid_extens_of_object($mod, $where));
				break;
			case 'queue':
				$mod = &$this->get_module('queuefeatures');
				return($this->_get_valid_extens_of_object($mod, $where));
				break;
			case 'meetme':
				$mod = &$this->get_module('meetmefeatures');
				return($this->_get_valid_extens_of_object($mod, $where, 'confno'));
			case 'incall':
				$mod = &$this->get_module('incall');
				return($this->_get_valid_extens_of_object($mod, $where, 'exten'));
				break;
			default:
				return(false);
		}
	}

	private function _get_valid_extens_of_object($module, $where, $key_exten='number')
	{
		$res = array();
		$elems = $module->get_all_where($where);
		foreach ($elems as $elem)
		{
			$exten = $elem[$key_exten];
			if (strlen($exten) > 0)
				array_push($res, (int) $exten);
		}
		return($res);
	}

	public function exten_is_available_for_context($exten, $context)
	{
		$mod = &$this->get_module('extensions');
		$where = array('context' => $context,
					   'exten' => $exten);

		if ($mod->get_nb($where) === 0)
			return(true);
		return(false);
	}
}

?>
