<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2011  Proformatique <technique@proformatique.com>
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
dwho::load_class('dwho_uri');
dwho::load_class('dwho_curl');
dwho::load_class('dwho_json');
dwho::load_class('dwho_file');

xivo_object::required(array('abstract','abstract.inc'),true);

abstract class xivo_object_provd_abstract extends xivo_object_abstract
{
	protected $_curl		= null;
	protected $_uri			= null;
	protected $_url			= array();
	protected $_status_code	= null;
	protected $_curl_opt	= array();
	
	protected function _load_init()
	{
		$this->_set_configuration();
		
		if (isset($this->_conf['provd']) === false)
			return(false);
		
		$this->_url = array('scheme' => 'http',
				    'host' => $this->_conf['provd']['host'],
				    'port' => $this->_conf['provd']['port']);

		$this->_uri = new dwho_uri();
		$this->_curl = new dwho_curl();
	}

	protected function _set_configuration()
	{
		$_CF = &dwho_gat::get('_CF');

		if(dwho_issa('provd',$_CF['xivo']['ini']) === false)
			return(null);

		$ref = &$_CF['xivo']['ini']['provd'];

		if(isset($ref['host']) === true)
		{
			if(dwho_network::valid_host($ref['host'],
						    (DWHO_NETWORK_HOST_IPV4_DOTDEC | DWHO_NETWORK_HOST_HOSTNAME)) === true)
				$this->_conf['provd']['host'] = $ref['host'];
			else
				dwho_report('warning','Invalid option host for section provd in XIVO init file');
		}

		if(isset($ref['port']) === true)
		{
			if(dwho_network::valid_port_number($ref['port']) === true)
				$this->_conf['provd']['port'] = (int) $ref['port'];
			else
				dwho_report('warning','Invalid option port for section provd in XIVO init file');
		}

		if(isset($ref['user']) === true)
		{
			if(empty($ref['user']) === false)
				$this->_conf['provd']['user'] = $ref['user'];
			else
				dwho_report('warning','Invalid option login for section provd in XIVO init file');
		}

		if(isset($ref['pass']) === true)
		{
			if(empty($ref['pass']) === false)
				$this->_conf['provd']['pass'] = $ref['pass'];
			else
				dwho_report('warning','Invalid option pass for section provd in XIVO init file');
		}

		if(isset($ref['connect_timeout']) === true)
		{
			if(dwho_is_uint($ref['connect_timeout']) === true)
				$this->_conf['provd']['connect_timeout'] = $ref['connect_timeout'];
			else
				dwho_report('warning','Invalid option connect_timeout for section provd in XIVO init file');
		}

		if(isset($ref['timeout']) === true)
		{
			if(dwho_is_uint($ref['timeout']) === true)
				$this->_conf['provd']['timeout'] = $ref['timeout'];
			else
				dwho_report('warning','Invalid option timeout for section provd in XIVO init file');
		}

		return(true);
	}

	protected function _set_curl_opt($key,$value)
	{		
		if (is_array($this->_curl_opt) === false)
			return(false);

		$this->_curl_opt[$key] = $value;

		return(true);
	}
	
	public function _build_uri($arr)
	{
		if (is_array($arr) === false
		|| ($nb = count($arr)) === 0)
			return($this->_base_url);
			
		$path = $this->_base_url.'/'.implode('/', $arr);
			
		if ($this->_uri->valid_path($path) === false)
			return(false);
		
		return($path);
	}
	
	public function decode_header($header)
	{
		$ret = array();		
		$fields = explode("\r\n", preg_replace('/\x0D\x0A[\x09\x20]+/', ' ', $header));
		foreach( $fields as $field ) 
		{
			if( preg_match('/([^:]+): (.+)/m', $field, $match) ) 
			{
				$match[1] = preg_replace('/(?<=^|[\x09\x20\x2D])./e', 'strtoupper("\0")', strtolower(trim($match[1])));
				if( isset($ret[$match[1]]) )
					$ret[$match[1]] = array($ret[$match[1]], $match[2]);
				else
					$ret[$match[1]] = trim($match[2]);
			}
		}
		
		return($ret);
	}
	
	protected function _split_top_parentheses($str)
	{
		$idx = 0;
	    $length = count($str);
	    $result = array();
	    while ($idx < $length)
	    {
	        if ($str[$idx] != '(')
	            dwho_report::push('error',sprintf('invalid character: %s', $str[$idx]));
			$start_idx = $idx;
			$idx += 1;
	        $count = 1;
	        while ($count)
	        {
	            if ($idx >= $length)
	            	dwho_report::push('error',sprintf('unbalanced number of parentheses: %s', $str));
	            $c = $str[$idx];
	            if ($c == '(')
	                $count += 1;
	            elseif ($c == ')')
	                $count -= 1;
	            $idx += 1;
	        }
	        $end_idx = $idx;
	        array_push($result,substr($str,$start_idx+1,$end_idx-1));
	    }
	    return $result;
	}

	protected function _create()
	{
		$opt = array('connect_timeout'	=> $this->_conf['provd']['connect_timeout'],
					'timeout'			=> $this->_conf['provd']['timeout'],
					#'writefunction'	=> array($this,'header'),
					#'followlocation'	=> true,
					#'httpheader'		=> array(dwho_json::get_header()));
					'httpheader'		=> array('Content-Type: text/plain'));
		
		if(is_array($this->_curl_opt) === true
		&& empty($this->_curl_opt) === false)
			$opt = array_merge($opt, $this->_curl_opt);
		
		if (($user = $this->_conf['provd']['user']) !== ''
		&& ($pass = $this->_conf['provd']['pass']) !== '')
			$opt = array_merge($opt, array('httpauth' => CURLAUTH_DIGEST,
				'userpwd' => $user.':'.$pass));
		
		return($this->_curl->load(null,$opt,false));
	}
	
	public function build_href($path,$query=null)
	{
		if(dwho_has_len($path) === false)
			return(false);

		$url = $this->_url;
		$url['path'] = $path;

		if(is_array($query) === true)
			$url['query'] = $query;

		if(($uri = $this->_uri->build_uri($url)) === false)
			return(false);
			
		return($uri);
	}

	protected function _connect($path,$query=null)
	{
		if(($uri = $this->build_href($path,$query)) === false
		|| $this->_curl->set_option('url',$uri) === false)
			return(false);

		return($this->_curl->connect());
	}

	public function last_status_code()
	{
		return($this->_status_code);
	}
	
	public function edit_params($uri,$value)
	{		
		$data = array();
		$data['param'] = array();
		$data['param']['value'] = $value;
		
		if ($this->request_put($uri,$data) === false
		|| $this->_status_code !== 204)
			return(false);
			
		return(true);
	}
	
	public function get_params($uri)
	{		
		if (($rs = $this->request_get($uri)) === false
		|| empty($rs['param']) === true)
			return(false);
			
		return($rs['param']);
	}
	
	public function request_oip($path)
	{
		$this->_status_code = null;

		if($this->_create() === false)
			return(false);

		$this->_curl->set_option('get',true);

		if(($r = $this->_connect($path,$query)) === false)
			return(false);

		$this->_status_code = $this->_curl->get_info('http_code');
		$this->_curl->close();

		return($r);
	}

	public function request_get($path,$query=null,$decode_assoc=true)
	{
		$this->_status_code = null;

		if($this->_create() === false)
			return(false);

		$this->_curl->set_option('get',true);

		if(($r = $this->_connect($path,$query)) !== false
		&& $r !== '')
		{
			if (($decoded = dwho_json::decode(rtrim($r),$decode_assoc)) !== null)
			    $r = $decoded;
		}

		$this->_status_code = $this->_curl->get_info('http_code');
		$this->_curl->close();

		return($r);
	}

	public function request_delete($path,$query=null,$decode_assoc=true)
	{
		$this->_status_code = null;

		if($this->_create() === false)
			return(false);

		$this->_curl->set_option('customrequest','DELETE');
		$this->_curl->set_option('get',true);

		if(($r = $this->_connect($path,$query)) !== false
		&& $r !== '')
			$r = dwho_json::decode(rtrim($r),$decode_assoc);

		$this->_status_code = $this->_curl->get_info('http_code');
		$this->_curl->close();

		return($r);
	}

	function request_put($path,$data,$query=null,$decode_assoc=true)
	{
		$this->_status_code = null;

		if($this->_create() === false)
			return(false);

		$this->_curl->set_option('customrequest','PUT');
		$this->_curl->set_option('post',true);
		$this->_curl->set_option('postfields',dwho_json::encode($data));

		if(($r = $this->_connect($path,$query)) !== false
		&& $r !== '')
		{
			if (($decoded = dwho_json::decode(rtrim($r),$decode_assoc)) !== null)
			    $r = $decoded;
		}

		$this->_status_code = $this->_curl->get_info('http_code');
		$this->_curl->close();

		return($r);
	}

	function request_post($path,$data,$query=null,$decode_assoc=true)
	{
		$this->_status_code = null;

		if($this->_create() === false)
			return(false);

		$this->_curl->set_option('post',true);
		$this->_curl->set_option('postfields',dwho_json::encode($data));

		if(($r = $this->_connect($path,$query)) !== false
		&& $r !== '')
		{
			if (($decoded = dwho_json::decode(rtrim($r),$decode_assoc)) !== null)
			    $r = $decoded;
		}

		$this->_status_code = $this->_curl->get_info('http_code');
		$this->_curl->close();

		return($r);
	}
}

?>