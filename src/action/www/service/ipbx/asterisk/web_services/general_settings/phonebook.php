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
$access_category = 'general_settings';
$access_subcategory = 'phonebook';

include(dwho_file::joinpath(dirname(__FILE__),'..','_common.php'));


$appaccessfeatures = &$ipbx->get_application('accessfeatures',array('feature' => 'phonebook'));
$info['accessfeatures'] = $appaccessfeatures->get();

$appldapfilter = $ipbx->get_application('serverfeatures',array('feature' => 'phonebook','type' => 'ldap'));
$info['ldapfilter'] = $appldapfilter->get();
$info['ldapfilter'] = $appldapfilter->get_server_list();

$act = $_QRY->get('act');

switch($act)
{
	case 'view':
	default:

		$_TPL->set_var('info',$info);
		break;

}

$_TPL->set_var('act',$act);
$_TPL->set_var('sum',$_QRY->get('sum'));
$_TPL->display('/service/ipbx/'.$ipbx->get_name().'/generic');

?>
