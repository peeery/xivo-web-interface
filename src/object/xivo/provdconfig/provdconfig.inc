<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2016 Avencall
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

dwho::load_class('dwho_network');

xivo_object::required(array('abstract','provd.inc'),true);

class xivo_object_provdconfig extends xivo_object_provd_abstract
{
	protected $_name 			= 'provdconfig';
	protected $_base_url 		= '/provd/cfg_mgr';
	protected $_internal 		= array('base','autoprov');
	protected $_confdefault 	= false;
	protected $_device_api		= false;
	protected $_extensions		= false;
	protected $_sccpdevice		= false;
	protected $_linefeatures	= false;
	protected $_userfeatures	= false;
	protected $_phonefunckey 	= false;
	protected $_appsip			= false;
	protected $_user_line 		= false;
	protected $_service			= false;
	protected $_func_key		= false;

	public function __construct(&$xobj,$param=array())
	{
		if(($_SRE = dwho_gct::get('xivo_service')) === false
		|| ($this->_service = &$_SRE->get('ipbx')) === false
		|| ($this->_extensions = &$this->_service->get_module('extensions')) === false
		|| ($this->_sccpdevice = &$this->_service->get_module('sccpdevice')) === false
		|| ($this->_linefeatures = &$this->_service->get_module('linefeatures')) === false
		|| ($this->_userfeatures = &$this->_service->get_module('userfeatures')) === false
		|| ($this->_phonefunckey = &$this->_service->get_module('phonefunckey')) === false
		|| ($this->_user_line = &$this->_service->get_module('user_line')) === false
		|| ($this->_func_key = &$this->_service->get_module('func_key')) === false
		|| ($this->_appsip = &$this->_service->get_apprealstatic('sip')) === false
		|| ($_RAPI = dwho_gct::get('xivo_confd')) === false
		|| ($this->_device_api = $_RAPI->get_ressource('device')) === false)
			return(false);

		$this->_load_config(true);
		$this->_load_init();
	}

	public function get_supp_elements()
	{
		$dir = dwho_file::joinpath(dirname(__FILE__),'config');

		if(($listfile = dwho_file::read_d($dir)) === false
		|| ($nb = count($listfile)) === 0)
			return(false);

		$res = array();

		for($i=0;$i<$nb;$i++)
		{
			$ref = &$listfile[$i];
			if(($info = dwho_file::get_info($dir.DWHO_SEP_DIR.$ref)) === false
			|| ($conf = dwho_gat::get_file($dir.DWHO_SEP_DIR.$info['filename'])) === false)
				continue;

			$res[$info['filename']] = $conf['element'];
		}

		return($res);
	}

	public function _prepare($data)
	{
		return($data);
	}

	public function get_default()
	{
		if (is_array($this->_confdefault) === true)
			return($this->_confdefault);
		elseif ($this->_confdefault === false
		&& ($confdefault = $this->get('default')) === false
		&& $this->eval_required_config(null,true) === false)
		{
			dwho_report::push('error', 'fatal error: config default not exist.');
			return(false);
		}

		return(($this->_confdefault = $confdefault));
	}

	public function get($id)
	{
		$this->reset();

		if(dwho_has_len($id) === false)
			return(false);

		$uri = $this->_build_uri(array('configs',$id));
		$rs = $this->request_get($uri);

		if($this->_status_code !== 200
		|| empty($rs['config']) === true)
			return(false);

		return($rs['config']);
	}

	public function get_configs($mode,$search='',$order=null,$limit=null,$assoc=false,$internal=false,$type=null)
	{
		$this->reset();

		switch ($mode)
		{
			default:
			case 'list':
				$uri = $this->_build_uri(array('configs'));
		}

		$this->set_order($order);
		$this->set_limit($limit);

		$query = array();
		$q = array();
		switch ($type)
		{
			case 'registrar':
				$q = dwho_json::encode(array('X_type' => 'registrar'));
				break;
			case 'device':
				$q = dwho_json::encode(array('X_type' => 'device'));
				break;
			case 'all':
			default:
				$w = array('$nin' => array('registrar','device'));
				$q = dwho_json::encode(array('X_type' => $w));
		}
		array_push($query,array('q64',base64_encode($q)));

		$rs = $this->request_get($uri,$query);

		if (is_array($rs) === false
		|| ($rs = $rs['configs']) === false
		|| ($nb = count($rs)) === 0)
			return(false);

		$this->_cnt = $nb;

		for($i=0;$i<$nb;$i++)
		{
			$ref = &$rs[$i];

			if (in_array($ref['id'],$this->_internal) === true)
			{
				unset($rs[$i]);
				continue;
			}

			if ($search !== ''
			&& preg_match('/'.$search.'/', $ref['id']) === 0)
			unset($rs[$i]);
		}

		if((bool) $assoc === false)
			return(array_values($rs));

		return($rs);
	}

	public function add($arr)
	{
		if (is_array($arr) === false)
			return(false);

		if (isset($arr['deletable']) === false)
			$arr['deletable'] = true;

		$uri = $this->_build_uri(array('configs'));
		$data['config'] = $arr;
		$rs = $this->request_post($uri,$data);

		if($this->_status_code !== 201)
		{
			dwho_report::push('error', 'fail to create config');
			return(false);
		}

		return($rs['id']);
	}

	public function edit($id,$arr)
	{
		if (is_array($arr) === false
		|| dwho_has_len($id) === false)
			return(false);

		if ($id === 'default'
		|| $id === 'defaultconfigdevice')
			$arr['deletable'] = false;
		elseif (isset($arr['deletable']) === false)
			$arr['deletable'] = true;

		$uri = $this->_build_uri(array('configs',$id));
		$data['config'] = $arr;
		$rs = $this->request_put($uri,$data);

		if($this->_status_code !== 204)
		{
			dwho_report::push('error', 'fail to edit config: '.$id);
			return(false);
		}

		return(true);
	}

	public function delete($id)
	{
		if(dwho_has_len($id) === false)
			return(false);

		$uri = $this->_build_uri(array('configs',$id));
		$rs = $this->request_delete($uri);

		if($this->_status_code !== 204)
			return(false);

		return(true);
	}

	public function autocreate()
	{
		$uri = $this->_build_uri(array('autocreate'));
		$rs = $this->request_post($uri,array('' => ''));

		if($this->_status_code !== 201)
		{
			dwho_report::push('error', 'fail to create autocreate config');
			return(false);
		}

		return($rs['id']);
	}

	private function _delete_required_config()
	{
		if ($this->get('default') !== false)
			$this->delete('default');
		if ($this->get('base') !== false)
			$this->delete('base');
		if ($this->get('autoprov') !== false)
			$this->delete('autoprov');
		if ($this->get('defaultconfigdevice') !== false)
			$this->delete('defaultconfigdevice');
	}

	public function rebuild_required_config($confdefault=null)
	{
		if (is_null($confdefault) === true)
			$confdefault = $this->get('default');

		if ($this->_add_base($confdefault) === false
		|| $this->_add_autoprov($confdefault) === false
		|| $this->_add_configdevice($confdefault) === false)
			return(false);

		return(true);
	}

	public function eval_required_config($registrarip=null,$reset=false)
	{
		$confdefault = false;

		if ($reset === true)
		{
			$this->_delete_required_config();
			if (is_null($registrarip) === true)
				$registrarip = $_SERVER['SERVER_ADDR'];
			return($this->eval_required_config($registrarip));
		}

		if (is_null($registrarip) === false)
		{
			if (dwho_network::valid_ipv4($registrarip) === false
			|| $this->_add_default_registrar_config($registrarip) === false
			|| is_array($this->_confdefault) === false)
			{
				dwho_report::push('error', 'fail to create default registrar config');
				return(false);
			}
			$confdefault = $this->_confdefault;
		}
		elseif(($confdefault = $this->get('default')) === false
		|| $this->eval_required_config(null,true) === false
		|| $this->eval_required_config() === false)
		{
			dwho_report::push('error', 'invalid default registrar config');
			return(false);
		}

		if ($this->rebuild_required_config($confdefault) === false)
		{
			dwho_report::push('error', 'Can\'t rebuild config');
			return(false);
		}

		return(true);
	}

	private function _add_default_registrar_config($registrarip)
	{
		$config = array();
		$config['X_type'] = 'registrar';
		$config['id'] = 'default';
		$config['deletable'] = false;
		$config['displayname'] = 'local';
		$config['parent_ids'] = array();
		// required raw_config dict by provd
		$config['raw_config'] = array('X_key' => 'xivo');
		$config['proxy_main'] = $registrarip;
		$config['registrar_main'] = $registrarip;

		if ($this->chk_confregistrar($config) === false
		|| $this->add($config) === false)
		{
			dwho_report::push('error', 'fail to create default registrar config');
			return(false);
		}

		$this->_confdefault = $config;

		return(true);
	}

	private function _add_base($confregistrar)
	{
		$base = $this->_conf['base'];

		if ($this->chk_confregistrar($confregistrar) === false)
			return(false);

		$base['X_type'] = 'internal';
		$base['deletable'] = false;

		$base['raw_config']['ntp_enabled'] = true;
		$base['raw_config']['ntp_ip'] = $confregistrar['proxy_main'];
		$base['raw_config']['X_xivo_phonebook_ip'] = $confregistrar['proxy_main'];

		if (($rs = $this->get('base')) !== false)
			$this->delete('base');

		if (is_array($rs) === true)
			$base['raw_config'] = array_merge($rs['raw_config'],$base['raw_config']);

		if ($this->add($base) === false)
		{
			dwho_report::push('error', 'fail to create base config');
			return(false);
		}

		return(true);
	}

	private function _add_autoprov($confregistrar)
	{
		$autoprov = $this->_conf['autoprov'];

		if ($this->chk_confregistrar($confregistrar) === false)
			return(false);

		if(($_SRE = dwho_gct::get('xivo_service')) !== false
		&& ($ipbx = &$_SRE->get('ipbx')) !== false)
		{
			$appsip = &$ipbx->get_apprealstatic('sip');
			$appgeneralsip = &$appsip->get_module('general');
			if(($rs = $appgeneralsip->get('autocreate_prefix')) === false
			|| isset($rs['general'], $rs['general']['var_val']) === false)
				dwho_report::push('error', 'Can\'t get autocreate_prefix in staticsip table');
			else
				$autoprov['raw_config']['sip_lines'][1]['username'] = $rs['general']['var_val'];
		}

		$autoprov['X_type'] = 'internal';
		$autoprov['deletable'] = false;
		$autoprov['raw_config']['sip_lines'][1]['proxy_ip'] = $confregistrar['proxy_main'];
		$autoprov['raw_config']['sip_lines'][1]['registrar_ip'] = $confregistrar['registrar_main'];

		$autoprov['raw_config']['sccp_call_managers'][1] = array();
		$autoprov['raw_config']['sccp_call_managers'][1]['ip'] = $confregistrar['proxy_main'];

		if (empty($confregistrar['proxy_backup']) === false)
		{
			$autoprov['raw_config']['sip_lines'][1]['backup_registrar_ip'] = $confregistrar['proxy_backup'];
			$autoprov['raw_config']['sip_lines'][1]['backup_proxy_ip'] = $confregistrar['proxy_backup'];
			$autoprov['raw_config']['sccp_call_managers'][2] = array();
			$autoprov['raw_config']['sccp_call_managers'][2]['ip'] = $confregistrar['proxy_backup'];
		}

		if (($rs = $this->get('autoprov')) !== false)
			$this->delete('autoprov');
		if (is_array($rs) === true)
			$autoprov['raw_config'] = array_merge($rs['raw_config'],$autoprov['raw_config']);

		if ($this->add($autoprov) === false)
		{
			dwho_report::push('error', 'fail to create autoprov config');
			return(false);
		}

		return(true);
	}

	private function _add_configdevice($confregistrar)
	{
		if ($this->chk_confregistrar($confregistrar) === false)
			return(false);

		$conf = array();
		$conf['X_type'] = 'device';
		$conf['deletable'] = false;
		$conf['id'] = 'defaultconfigdevice';
		$conf['label'] = 'Default config device';
		$conf['parent_ids'] = array();
		$conf['raw_config'] = array();
		$conf['raw_config']['ntp_enabled'] = true;
		$conf['raw_config']['ntp_ip'] = $confregistrar['proxy_main'];
		$conf['raw_config']['sip_dtmf_mode'] = 'SIP-INFO';

		if (($rs = $this->get('defaultconfigdevice')) !== false)
			$this->delete('defaultconfigdevice');
		if (is_array($rs) === true)
			$conf['raw_config'] = array_merge($rs['raw_config'],$conf['raw_config']);

		if ($this->add($conf) === false)
		{
			dwho_report::push('error', 'fail to create default config device');
			return(false);
		}

		return(true);
	}

	public function get_exten($name)
	{
		if($this->_extensions === false
		|| ($rs = $this->_extensions->get_where(array('typeval' => $name))) === false)
			return(null);

		$pattern_symbols = array('_', '.', '!', 'X', 'Z', 'N');
		$exten = $rs['exten'];
		foreach ($pattern_symbols as $symbol)
        {
			$exten = str_replace($symbol, '', $exten);
        }

		return($exten);
	}

	private function _set_config_var(&$config,$data,$var,$type=null)
	{
		if (isset($data[$var]) === true
		&& dwho_empty($data[$var]) === false)
		{
			switch ($type)
			{
				case 'int':
					$rs = (int) ($data[$var]);
					break;
				case 'bool':
					$rs = dwho_bool($data[$var]);
					break;
				case null:
				default:
					$rs = $data[$var];
			}
			$config[$var] = $rs;
		}
		else
			unset($config[$var]);
	}

	public function init_device_config($configid)
	{
		if (($globalconfig = $this->get($configid)) === false)
		{
			$globalconfig = array();
			$globalconfig['id'] = $configid;
			$globalconfig['configdevice'] = 'defaultconfigdevice';
			$globalconfig['parent_ids'] = array('base','defaultconfigdevice');
			$globalconfig['raw_config'] = array('X_key' => '');

			$this->add($globalconfig);
		}
		return($globalconfig);
	}

	private function _build_exten_config(&$config)
	{
		$config['exten_dnd'] = $this->get_exten('enablednd');
		$config['exten_fwd_unconditional'] = $this->get_exten('fwdunc');
		$config['exten_fwd_no_answer'] = $this->get_exten('fwdrna');
		$config['exten_fwd_busy'] = $this->get_exten('fwdbusy');
		$config['exten_fwd_disable_all'] = $this->get_exten('fwdundoall');
		$config['exten_park'] = $this->get_exten('parkext');
		$config['exten_pickup_group'] = $this->get_exten('pickupexten');
		$config['exten_pickup_call'] = $this->get_exten('pickup');
		$config['exten_voicemail'] = $this->get_exten('vmusermsg');
	}

	private function _build_funckeys(&$raw_config,$user_id)
	{
		$funckeys = array();
		if (($funckeys_v1 = $this->_phonefunckey->build_provd_conf_funckey($user_id)) !== false)
			$funckeys = $funckeys + $funckeys_v1;
		if (($funckeys_v2 = $this->_func_key->get_provd_conf($user_id)) !== false)
			$funckeys = $funckeys + $funckeys_v2;

		if (empty($funckeys))
			unset($raw_config['funckeys']);
		else
			$raw_config['funckeys'] = $funckeys;
	}

	private function _add_user_info(&$raw_config,$user_id,$main_line)
	{
		$user = $this->_userfeatures->get($user_id);
		if (empty($user['uuid']) === false) {
			$raw_config['X_xivo_user_uuid'] = $user['uuid'];
		}
		if (empty($main_line['context']) === false) {
			$raw_config['X_xivo_phonebook_profile'] = $main_line['context'];
		}
	}

	private function _build_lines(&$raw_config,$listline)
	{
		foreach ($listline as $linefeatures)
		{
			$protocolname = $linefeatures['protocol'];
			switch($protocolname)
			{
				case 'sip':
					$this->_build_sip_secure_config($raw_config);
					$this->_build_sip_config($raw_config, $linefeatures);
					break;
				case 'sccp':
					$this->_build_sccp_config($raw_config, $linefeatures);
					break;
			}
		}
	}

	private function _build_sipline($protoline, $confregistrar)
	{
		if ($this->chk_confregistrar($confregistrar) === false)
			return(false);

		$conf = array();
		$conf['proxy_ip'] = $confregistrar['proxy_main'];
		$conf['username'] = $protoline['name'];
		$conf['auth_username'] = $protoline['name'];
		$conf['password'] = $protoline['secret'];

		$calleridname = trim(preg_replace('/<[^<]*>$/','',trim(strval($protoline['callerid']))));
		$calleridname = str_replace('"','',str_replace('\"','"',$calleridname));
		$conf['display_name'] = $calleridname;

		$confregistrar['registrar_ip'] = $confregistrar['registrar_main'];
		$confregistrar['proxy_ip'] = $confregistrar['proxy_main'];
		if (empty($confregistrar['proxy_backup']) === false)
		{
			$confregistrar['backup_registrar_ip'] = $confregistrar['registrar_backup'];
			$confregistrar['backup_proxy_ip'] = $confregistrar['proxy_backup'];
		}

		$this->_set_config_var($conf,$confregistrar,'registrar_ip');
		$this->_set_config_var($conf,$confregistrar,'proxy_ip');
		$this->_set_config_var($conf,$confregistrar,'backup_proxy_ip');
		$this->_set_config_var($conf,$confregistrar,'backup_registrar_ip');
		$this->_set_config_var($conf,$protoline,'number');
		$this->_set_config_var($conf,$protoline,'dtmf_mode');
		$this->_set_config_var($conf,$protoline,'srtp_mode');
		$this->_set_config_var($conf,$protoline,'voicemail');

		return($conf);
	}

	private function _build_sip_config(&$raw_config, $linefeatures)
	{
		$raw_config['sip_dtmf_mode'] = 'SIP-INFO';
		$raw_config['protocol'] = 'SIP';

		if (isset($raw_config['sip_lines']) === false)
			$raw_config['sip_lines'] = array();

		$confregistrar = $this->get($linefeatures['configregistrar']);
		$line_number = $linefeatures['num'];

		if(($modprotocol = &$this->_service->get_protocol_module('sip')) === false
		|| ($protocol = $modprotocol->get($linefeatures['protocolid'])) === false)
			return(false);

		$protoline = array_merge($protocol, $linefeatures);

		$raw_config['sip_lines'][$line_number] = $this->_build_sipline($protoline, $confregistrar);
	}

	private function _build_sip_secure_config(&$config)
	{
		$appgeneralsip = &$this->_appsip->get_module('general');

		$tlsenable = $appgeneralsip->get('tlsenable');
		if (isset($config['sip_transport']) === true
		&& $config['sip_transport'] === 'tls'
		&& $tlsenable['general']['var_val'] === 'yes')
		{
			$tlscafile = $appgeneralsip->get('tlscafile');
			$crt = $tlscafile['general']['var_val'];
			$config['sip_servers_root_and_intermediate_certificates'] =
				file_get_contents($crt);
		}
		else
		{
			unset($config['sip_srtp_mode'],
			$config['sip_servers_root_and_intermediate_certificates']);

			if (isset($config['sip_transport']) === true
			&& $config['sip_transport'] === 'tls') {
				unset($config['sip_transport']);
			}
		}
	}

	private function _build_sccp_config(&$raw_config, $linefeatures)
	{
		$confregistrar = $this->get($linefeatures['configregistrar']);

		$raw_config['protocol'] = 'SCCP';
		$raw_config['sccp_call_managers'] = array();
		$raw_config['sccp_call_managers'][1] = array();
		$raw_config['sccp_call_managers'][1]['ip'] = $confregistrar['proxy_main'];
		if (empty($confregistrar['proxy_backup']) === false)
		{
			$raw_config['sccp_call_managers'][2] = array();
			$raw_config['sccp_call_managers'][2]['ip'] = $confregistrar['proxy_backup'];
		}
	}

	public function update_raw_config($configid, $new_raw_config)
	{
		$globalconfig = $this->init_device_config($configid);

		$raw_config = &$globalconfig['raw_config'];

		$raw_config_device = $this->build_raw_config_device($new_raw_config);
		$config = array_merge($raw_config,$raw_config_device);

		foreach ($config as $key => $val)
		{
			if (substr($key, 0, 5) === 'exten'
			|| $key === 'sip_lines'
			|| $key === 'funckeys'
			|| isset($raw_config_device[$key]))
				continue;

			unset($config[$key]);
		}

		$this->_build_sip_secure_config($config);

		if (empty($config) === true)
			$config = array('X_Key' => '');

		$globalconfig['raw_config'] = $config;

		if ($this->edit($configid,$globalconfig) === false)
		{
			dwho_report::push('error', 'update_raw_config -> fail to edit config: '.$configid);
			return(false);
		}
		return(true);
	}

	private function _get_config_device($configid)
	{
		if ($configid === null)
			$config = null;
		elseif (($config = $this->get($configid)) === false)
			$config = null;

		$configdevice = isset($config['configdevice']) === true ? $config['configdevice'] : 'defaultconfigdevice';

		return($configdevice);
	}

	private function _create_or_edit_config($configid,$config)
	{
		if ($this->get($configid) === false)
		{
			if ($this->add($config) === false)
			{
				dwho_report::push('error', 'could not create config '.$configid);
				return(false);
			}
		}
		else
		{
			if ($this->edit($configid, $config) === false)
			{
				dwho_report::push('error', 'could not edit config'.$configid);
				return(false);
			}
		}
	}

	public function rebuild_config($deviceid,$configid=null)
	{
		$where = array('device' => (string) $deviceid);
		$order = array('num' => SORT_ASC);
		$listline = $this->_linefeatures->get_all_where($where,null,true,$order);

		if ($listline === false)
		{
			if ($this->_device_api->reset_to_autoprov($deviceid) === false)
			{
				dwho_report::push('error', 'coud not reset to autoprov device '.$deviceid);
				return(false);
			}

			return(null);
		}

		$configdevice = $this->_get_config_device($configid);
		$user_id = $this->_user_line->get_master_user_id_by_line_id($listline[0]['id']);

		$newconfigid = $deviceid;
		$newconfig = array();
		$newconfig['id'] = $newconfigid;
		$newconfig['configdevice'] = $configdevice;
		$newconfig['parent_ids'] = array('base',$configdevice);
		$newconfig['raw_config'] = array('X_key' => '', 'config_version' => 1);
		$newrawconfig = &$newconfig['raw_config'];
		$this->_build_exten_config($newrawconfig);
		$this->_build_funckeys($newrawconfig,$user_id);
		$this->_build_lines($newrawconfig,$listline);
		$this->_add_user_info($newrawconfig,$user_id,$listline[0]);

		$this->_create_or_edit_config($newconfigid, $newconfig);

		return($newconfigid);
	}

	public function build_raw_config_device($raw)
	{
		$conf = array();

		$this->_set_config_var($conf,$raw,'X_key');
		$this->_set_config_var($conf,$raw,'_comment');
		$this->_set_config_var($conf,$raw,'timezone');
		$this->_set_config_var($conf,$raw,'locale');
		$this->_set_config_var($conf,$raw,'protocol');
		$this->_set_config_var($conf,$raw,'ntp_enabled','int');
		$this->_set_config_var($conf,$raw,'ntp_ip');
		$this->_set_config_var($conf,$raw,'vlan_enabled','int');
		$this->_set_config_var($conf,$raw,'vlan_id');
		$this->_set_config_var($conf,$raw,'vlan_priority');
		$this->_set_config_var($conf,$raw,'vlan_pc_port_id');
		$this->_set_config_var($conf,$raw,'syslog_enabled','int');
		$this->_set_config_var($conf,$raw,'syslog_ip');
		$this->_set_config_var($conf,$raw,'syslog_port');
		$this->_set_config_var($conf,$raw,'syslog_level');
		$this->_set_config_var($conf,$raw,'config_encryption_enabled','int');
		$this->_set_config_var($conf,$raw,'sip_proxy_ip');
		$this->_set_config_var($conf,$raw,'sip_proxy_port');
		$this->_set_config_var($conf,$raw,'sip_backup_proxy_ip');
		$this->_set_config_var($conf,$raw,'sip_backup_proxy_port');
		$this->_set_config_var($conf,$raw,'sip_registrar_port');
		$this->_set_config_var($conf,$raw,'sip_backup_registrar_ip');
		$this->_set_config_var($conf,$raw,'sip_backup_registrar_port');
		$this->_set_config_var($conf,$raw,'sip_outbound_proxy_ip');
		$this->_set_config_var($conf,$raw,'sip_outbound_proxy_port');
		$this->_set_config_var($conf,$raw,'sip_dtmf_mode');
		$this->_set_config_var($conf,$raw,'sip_srtp_mode');
		$this->_set_config_var($conf,$raw,'sip_transport');
		$this->_set_config_var($conf,$raw,'admin_username');
		$this->_set_config_var($conf,$raw,'admin_password');
		$this->_set_config_var($conf,$raw,'user_username');
		$this->_set_config_var($conf,$raw,'user_password');
		$this->_set_config_var($conf,$raw,'sip_servers_root_and_intermediate_certificates');
		$this->_set_config_var($conf,$raw,'sip_local_root_and_intermediate_certificates');
		$this->_set_config_var($conf,$raw,'sip_local_certificate');
		$this->_set_config_var($conf,$raw,'sip_local_key');
		$this->_set_config_var($conf,$raw,'sip_subscribe_mwi','int');
		$this->_set_config_var($conf,$raw,'X_xivo_phonebook_scheme');
		$this->_set_config_var($conf,$raw,'X_xivo_phonebook_ip');
		$this->_set_config_var($conf,$raw,'X_xivo_phonebook_port');

		return($conf);
	}

	public function chk_confregistrar($arr)
	{
		if (is_array($arr) === false
		|| isset($arr['id'],$arr['registrar_main'],
		$arr['proxy_main'],$arr['displayname']) === false
		|| dwho_network::valid_ipv4($arr['registrar_main']) === false
		|| dwho_network::valid_ipv4($arr['proxy_main']) === false)
		{
			dwho_report::push('error', 'config registrar not valid');
			return(false);
		}

		return($arr);
	}
}

?>
