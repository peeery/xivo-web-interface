<?php

#
# XiVO Web-Interface
# Copyright 2015-2017 The Wazo Authors  (see the AUTHORS file)
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

require_once(dwho_file::joinpath(XIVO_PATH_OBJECT,'auth','client.inc'));
require_once(dwho_file::joinpath(XIVO_PATH_OBJECT,'auth','token','token.inc'));

class xivo_authent_xivo
{
	private $client = null;
	private $token_resource = null;

	public function __construct()
	{
		if(($this->client = AuthClient::default_config()) === false) {
			trigger_error('Invalid AUTH object',E_USER_ERROR);
			return(false);
		}

		if(($this->token_resource = new TokenResource($this->client)) === false) {
			trigger_error('Invalid TokenResource',E_USER_ERROR);
			return(false);
		}
	}

	function chk($login,$pass)
	{
		$tk = null;
		if(isset($_SESSION['token'])) {
			$tk = $this->token_resource->get($_SESSION['token']);
		}
		if(!$tk) {
			$tk = $this->token_resource->create($login, $pass);
		}
		if(!$tk) {
			return(false);
		}

		$token = $tk['data']['token'];
		if ($token) {
			$_SESSION['token'] = $token;
			return(true);
		}

		return(false);
	}

	function logoff()
	{
		if(isset($_SESSION['token'])) {
			$this->token_resource->delete($_SESSION['token']);
			unset($_SESSION['token']);
		}
	}
}

?>
