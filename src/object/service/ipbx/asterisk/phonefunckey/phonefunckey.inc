<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2014  Avencall
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

xivo_service_asterisk::required(array('abstract','abstract.inc'),true);

class xivo_service_asterisk_phonefunckey extends xivo_service_asterisk_abstract
{
	var $_dso			= null;
	var $_name			= 'phonefunckey';
	var $_filter		= false;
	var $_origin		= false;
	var $_origin_list	= false;
	var $_type_list		= false;

	var $_modextensions	= false;

	public function __construct(&$sre,&$dso)
	{
		if(is_object($sre) === false)
			trigger_error('Invalid service in '.__CLASS__,E_USER_ERROR);

		if(is_object($dso) === false)
			trigger_error('Invalid datastorage in '.__CLASS__,E_USER_ERROR);

		$this->_sre = &$sre;
		$this->_dso = &$dso;

		if(($this->_modextensions = $this->_sre->get_module('extensions')) === false)
			trigger_error('Could not load module extensions in '.__CLASS__,E_USER_ERROR);

		$this->_load_config();
	}

	function _prepare($data)
	{
		$data['iduserfeatures'] = (int) $data['iduserfeatures'];
		$data['type'] = '';
		$data['typeval'] = '';
		$data['user'] = '';
		$data['group'] = '';
		$data['queue'] = '';
		$data['meetme'] = '';
		$data['custom'] = '';
		$data['extension'] = false;
		$data['supervision'] = (bool) $data['supervision'];
		$data['progfunckey'] = (bool) $data['progfunckey'];

		switch($data['typeextenumbers'])
		{
			case 'extenfeatures':
				switch($data['typevalextenumbers'])
				{
					case 'bsfilter':
						$data['type'] = $data['typeextenumbers'].'-'.$data['typevalextenumbers'];
						$data['typeval'] = $data['typevalextenumbersright'];
						return($data);
					case 'agentstaticlogin':
					case 'agentstaticlogoff':
					case 'agentstaticlogtoggle':
						$data['type'] = $data['typeextenumbers'].'-'.$data['typevalextenumbers'];
						$data['typeval'] = $data['typevalextenumbersright'];
						return($data);
				}
			case 'featuremap':
			case 'generalfeatures':
				if($data['type'] === '')
					$data['type'] = $data['typeextenumbers'].'-'.$data['typevalextenumbers'];

				if(dwho_issa($data['type'],$this->_conf['type']) === true)
				{
					$data['extension'] = (bool) dwho_ak('extension',$this->_conf['type'][$data['type']],true);

					if((bool) dwho_ak('destination',$this->_conf['type'][$data['type']],true) === true)
						$data['typeval'] = $data['exten'];
				}
				return($data);
		}

		switch($data['typeextenumbersright'])
		{
			case 'user':
			case 'group':
			case 'queue':
			case 'paging':
			case 'meetme':
				$data['type'] = $data['typeextenumbersright'];
				$data[$data['typeextenumbersright']] = $data['typeval'] = $data['typevalextenumbersright'];
				return($data);
		}

		if(dwho_has_len($data['exten']) === true || $data['type'] === '')
		{
			$data['type'] = 'custom';
			$data['custom'] = $data['typeval'] = $data['exten'];
		}

		return($data);
	}

	function add($arr,$id=0)
	{
		if(is_array($arr) === false
		|| isset($arr['iduserfeatures']) === false
		|| ((int) $arr['iduserfeatures']) === 0)
			return(false);

		if (($rs = parent::add($arr,$id)) === false)
			return(false);

		return($rs);
	}

	function edit($id,$arr,$disable=null)
	{
		if(is_array($arr) === false)
			return(false);

		return(parent::edit($id,$arr,$disable));
	}

	function get_type_list()
	{
		return($this->_conf['type']);
	}

	function mk_values_array($arr)
	{
		if(is_array($arr) === false
		|| isset($arr['fknum'],$arr['type'],$arr['typeval'],$arr['iduserfeatures']) === false
		|| ($type_info = $this->_chk_exists_cfg('type',$arr['type'],true)) === false)
			return(false);

		$r = array();
		$r['iduserfeatures'] = (int) $arr['iduserfeatures'];
		$r['fknum'] = $arr['fknum'];
		$r['exten'] = '';
		$r['typeextenumbers'] = '';
		$r['typevalextenumbers'] = '';
		$r['typeextenumbersright'] = '';
		$r['typevalextenumbersright'] = '';

		if(isset($arr['label']) === true)
			$r['label'] = $arr['label'];
		else
			$r['label'] = '';

		if(isset($type_info['supervisable']) === true
		&& (bool) $type_info['supervisable'] === true
		&& isset($arr['supervision']) === true)
			$r['supervision'] = intval((bool) $arr['supervision']);
		else
			$r['supervision'] = 0;

		if(isset($type_info['progfunckey']) === true)
			$r['progfunckey'] = intval((bool) $type_info['progfunckey']);
		else
			$r['progfunckey'] = 0;

		$type = explode('-',$arr['type'],2);

		switch($arr['type'])
		{
			case 'extenfeatures-agentstaticlogin':
			case 'extenfeatures-agentstaticlogoff':
			case 'extenfeatures-agentstaticlogtoggle':
				if(dwho_has_len($arr['typeval']) === false)
					return(false);
				$r['typeextenumbers'] = 'extenfeatures';
				$r['typevalextenumbers'] = $type[1];
				$r['typeextenumbersright'] = substr($type[1],0,5);
				$r['typevalextenumbersright'] = $arr['typeval'];
				return($r);
			case 'extenfeatures-bsfilter':
				if(dwho_has_len($arr['typeval']) === false)
					return(false);

				$r['typeextenumbers'] = 'extenfeatures';
				$r['typevalextenumbers'] = $type[1];
				$r['typeextenumbersright'] = 'user';
				$r['typevalextenumbersright'] = $arr['typeval'];
				return($r);
			case 'extenfeatures-paging':
				if(dwho_has_len($arr['typeval']) === false)
					return(false);

				$r['typeextenumbers'] = 'extenfeatures';
				$r['typevalextenumbers'] = $type[1];
				$r['typeextenumbersright'] = 'paging';
				$r['typevalextenumbersright'] = $arr['typeval'];
				return($r);
		}

		switch($type[0])
		{
			case 'extenfeatures':
			case 'generalfeatures':
			case 'featuremap':
				if(isset($type_info['destination']) === true
				&& (bool) $type_info['destination'] === true)
					$r['exten'] = $arr['typeval'];

				$r['typeextenumbers'] = $type[0];
				$r['typevalextenumbers'] = $type[1];
				break;
			case 'custom':
				if(dwho_has_len($arr['typeval']) === false)
					return(false);

				$r['exten'] = $arr['typeval'];
				break;
			default:
				if(dwho_has_len($arr['typeval']) === false)
					return(false);

				$r['typeextenumbersright'] = $arr['type'];
				$r['typevalextenumbersright'] = $arr['typeval'];
		}

		return($r);
	}

	function is_valid($type='',$typeval='',$typeright='',$typevalright='',$id=null,$disable=null)
	{
		switch($type)
		{
			case 'extenfeatures':
				switch($typeval)
				{
					case 'paging':
					case 'bsfilter':
						return(true);
					case 'agentstaticlogin':
					case 'agentstaticlogoff':
					case 'agentstaticlogtoggle':
						if($typeright !== substr($typeval,0,5))
							return(false);
						break;
				}
			case 'featuremap':
			case 'generalfeatures':
				if($this->_chk_exists_cfg('type',$type.'-'.$typeval) === false)
					return(false);
				else if(dwho_has_len($typeright) === false)
					return(true);
				break;
			default:
				if(dwho_has_len($type) === true)
					return(false);
		}

		switch($typeright)
		{
			case 'agent':
				if($type !== 'extenfeatures'
				|| ($typeval !== 'agentstaticlogin'
				   && $typeval !== 'agentstaticlogoff'
				   && $typeval !== 'agentstaticlogtoggle') === true)
					return(false);

				return($this->chk_dest_agent($typevalright,null,null,$disable,false,true));
			case 'user':
				return($this->chk_dest_user($typevalright,null,$id,$disable,false,true));
			case 'group':
				return($this->chk_dest_group($typevalright,null,null,$disable,false,true));
			case 'queue':
				return($this->chk_dest_queue($typevalright,null,null,$disable,false,true));
			case 'meetme':
				return($this->chk_dest_meetme($typevalright,null,null,$disable,false,true));
		}

		return(false);
	}

	function get_type($type='',$typeval='',$typeright='',$typevalright='',$id=null,$disable=null)
	{
		$rs = array();
		switch($type)
		{
			case 'extenfeatures':
				switch($typeval)
				{
					case 'paging':
					case 'bsfilter':
						$rs['type'] = $type;
						$rs['val'] = $typeval;
						return($rs);
					case 'agentstaticlogin':
					case 'agentstaticlogoff':
					case 'agentstaticlogtoggle':
						if($typeright !== substr($typeval,0,5))
							return(false);
						break;
				}
			case 'featuremap':
			case 'generalfeatures':
				if($this->_chk_exists_cfg('type',$type.'-'.$typeval) === false)
					return(false);
				else if(dwho_has_len($typeright) === false)
				{
					$rs['type'] = $type;
					$rs['val'] = $typeval;
					return($rs);
				}
				break;
			default:
				if(dwho_has_len($type) === true)
					return(false);
		}

		switch($typeright)
		{
			case 'agent':
				if($type !== 'extenfeatures'
				|| ($typeval !== 'agentstaticlogin'
				   && $typeval !== 'agentstaticlogoff'
				   && $typeval !== 'agentstaticlogtoggle') === true)
					return(false);

				$rs['type'] = $typeright;
				$rs['val'] = $typevalright;
				break;
			case 'user':
			case 'queue':
			case 'group':
			case 'meetme':
				$rs['type'] = $typeright;
				$rs['val'] = $typevalright;
				break;
		}

		return($rs);

	}

	function clean_extension($exten) {
		if ($exten === null) {
			return('');
		}

		$exten = (string) $exten;

		if (substr($exten, 0, 1) === '_') {
			$exten = substr($exten, 1);
		}
		if (preg_match('/[\[NXZ!\.]/',$exten,$out) > 0) {
			$pos = strpos($exten, $out[0]);
			$exten = substr($exten, 0, $pos);
		}

		return($exten);
	}

	function fkey_extension($xleft, $xlist) {
		$components = array();

		$xleft = $this->clean_extension($xleft);

		foreach ($xlist as $x) {
			$x = $this->clean_extension($x);
			if (!empty($x)) {
				array_push($components,$x);
			}
		}

		return($xleft . $this->unsplit_extension($components));
	}

	/*
	 Compute and return an extension from multi extensions.

	>>> unsplit_extension(('*98','666'))
	'**198*666'
	*/
	function unsplit_extension($xlist) {
		if (!is_array($xlist)) {
			return(false);
		}

		$ret = array();
		$cur = '';
		$i = 0;
		foreach ($xlist as $x) {
			$i += 1;
			$arr = str_split($x, 1);
			foreach ($arr as $c) {
				if ($c === '*') {
					$cur .= sprintf('**%d', $i);
				} else {
					$cur .= $c;
				}
			}
			array_push($ret, $cur);
			$cur = '';
		}

		return(implode('*',$ret));
	}

	public function build_provd_conf_funckey($iduserfeatures)
	{
		$conf = array();

		if (($ipbx = &$this->_sre) === false
		|| ($modcallfiltermember = &$ipbx->get_module('callfiltermember')) === false
		|| ($moduserfeatures = &$ipbx->get_module('userfeatures')) === false
		|| ($appfeatures = &$ipbx->get_apprealstatic('features')) === false
		|| ($appfeaturemap = &$appfeatures->get_module('featuremap')) === false)
			return(false);

		$where = array();
		$where['iduserfeatures'] = $iduserfeatures;
		if (($rs = $this->get_all_where($where, null,true, array('fknum' => SORT_ASC))) === false
		|| ($nb = count($rs)) === 0)
			return(false);

		for($i = 0;$i < $nb;$i++)
		{
			$ref = &$rs[$i];

			$type = $this->get_type($ref['typeextenumbers'],
					$ref['typevalextenumbers'],
					$ref['typeextenumbersright'],
					$ref['typevalextenumbersright']);

			$conf[$ref['fknum']] = array();
			$conf[$ref['fknum']]['type'] = ($ref['supervision'] === false) ? 'speeddial' : 'blf';
			$conf[$ref['fknum']]['line'] = 1;
			$label = $ref['label'];
			if($label !== null)
				$conf[$ref['fknum']]['label'] = $label;

			if(empty($type) === true) {
				$conf[$ref['fknum']]['value'] = $ref['exten'];
				continue;
			}

			$exten = '';
			switch ($type['type'])
			{
				case 'generalfeatures':
					if ($type['val'] === 'parkext')
						$conf[$ref['fknum']]['type'] = 'park';
					$exten = $ref['exten'];
					break;
				case 'agent':
					if ($ref['progfunckey'] === true) {

						if ($ref['exten'] === null && $ref['typevalextenumbersright'] !== null)
							$typevalright = '*'.$ref['typevalextenumbersright'];

						$where = array('typeval' => 'phoneprogfunckey');
						if (($rsextenmbers = $this->_modextensions->get_where($where)) === false)
							break;
						$where = array();
						$where['type'] = $ref['typeextenumbers'];
						$where['typeval'] = $ref['typevalextenumbers'];
						if (($extenumleftexten = $this->_modextensions->get_where($where)) === false)
							break;

						$exten = $this->fkey_extension($rsextenmbers['extenbrut'],
								array($ref['iduserfeatures'],
										$extenumleftexten['extenbrut'],
										$typevalright)
						);
					}
					break;
				case 'featuremap':
					if (($rsextenmbers = $appfeaturemap->get($type['val'])) === false)
					{
						dwho_report::push('error', 'error during get featuremap: '.$type['val']);
						break;
					}
					$rsexten = $rsextenmbers['featuremap']['var_val'];
					$exten = str_replace('.', $callfiltermember['id'], $rsexten);
					break;
				case 'extenfeatures':
					$where = array();
					$where['type'] = $type['type'];
					$where['typeval'] = $type['val'];

					if (($rsextenmbers = $this->_modextensions->get_where($where)) !== false)
						$rsexten = $rsextenmbers['exten'];

					switch($type['val'])
					{
						case 'bsfilter':
							$userfeatures = $moduserfeatures->get($iduserfeatures);
							$bstype = $userfeatures["bsfilter"];

							switch ($bstype)
							{
								case 'boss':
									$boss_id = (string) $iduserfeatures;
									$secretary_id = (string) $ref['typevalextenumbersright'];
									break;
								case 'secretary':
									$secretary_id = (string) $iduserfeatures;
									$boss_id = (string) $ref['typevalextenumbersright'];
									break;
							}

							if (($callfiltermember = $modcallfiltermember->get_by_boss_secretary($boss_id, $secretary_id)) === false)
							{
								dwho_report::push('error', 'error during get callfiltermember of user');
								break;
							}

							$exten = str_replace('.', $callfiltermember['id'], $rsexten);
							break;
						default:
							if ($ref['progfunckey'] === true)
							{
								$exten = $this->build_progfunckey($iduserfeatures, $rsexten, $ref['exten']);
							} else {
								$exten = $this->build_extenfeatures($rsexten, $ref['typevalextenumbersright']);
							}
					}
			}

			$conf[$ref['fknum']]['value'] = $exten;
		}

		return((empty($conf) ? false : $conf));
	}

	public function build_progfunckey($user_id, $prefix_exten, $funckey_exten='') {
		$where = array('typeval' => 'phoneprogfunckey');
		if (($progfunckey = $this->_modextensions->get_where($where)) === false)
			return '';
		$exten = $this->fkey_extension($progfunckey['extenbrut'], array($user_id, $prefix_exten, $funckey_exten));
		return $exten;
	}

	public function build_extenfeatures($prefix_exten, $value='') {
		$exten = str_replace('.', $value, $prefix_exten);
		return $exten;
	}

}

?>
