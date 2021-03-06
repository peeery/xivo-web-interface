<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2016  Avencall
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

$array = array();
$array['element'] = array();

$array['element']['number'] = array();
$array['element']['number']['default'] = '';

$array['element']['duplex'] = array();
$array['element']['duplex']['value'] = array(0,1);
$array['element']['duplex']['default'] = 0;
$array['element']['duplex']['set'] = true;

$array['element']['ignore'] = array();
$array['element']['ignore']['value'] = array(0,1);
$array['element']['ignore']['default'] = 0;
$array['element']['ignore']['set'] = true;

$array['element']['record'] = array();
$array['element']['record']['value'] = array(0,1);
$array['element']['record']['default'] = 0;
$array['element']['record']['set'] = true;

$array['element']['quiet'] = array();
$array['element']['quiet']['value'] = array(0,1);
$array['element']['quiet']['default'] = 0;
$array['element']['quiet']['set'] = true;

$array['element']['timeout'] = array();
$array['element']['timeout']['value'] = array(0,15,30,45,60,75,90,
						     105,120,135,150,165,180,195,
						     210,225,240,255,270,285,300);
$array['element']['timeout']['default'] = 30;
$array['element']['timeout']['setdef'] = true;

$array['element']['announcement_file'] = array();
$array['element']['announcement_file']['default'] = '';

$array['element']['announcement_play'] = array();
$array['element']['announcement_play']['value'] = array(0,1);
$array['element']['announcement_play']['default'] = 0;
$array['element']['announcement_play']['set'] = true;

$array['element']['announcement_caller'] = array();
$array['element']['announcement_caller']['value'] = array(0,1);
$array['element']['announcement_caller']['default'] = 0;
$array['element']['announcement_caller']['set'] = true;

$array['element']['commented'] = array();
$array['element']['commented']['value'] = array(0,1);
$array['element']['commented']['default'] = 0;
$array['element']['commented']['set'] = true;

$array['element']['description'] = array();
$array['element']['description']['default'] = '';


$array['filter'] = array();
$array['filter']['number'] = array('minlen' => 1,'maxlen' => 40,'regexp' =>  '/^[0-9]+$/');
$array['filter']['duplex'] = array('bool' => true);
$array['filter']['ignore'] = array('bool' => true);
$array['filter']['record'] = array('bool' => true);
$array['filter']['quiet'] = array('bool' => true);
$array['filter']['timeout'] = array('set'	=> false,
								   'chk'	=> 2,
								   'key' 	=> $array['element']['timeout']['value']);
$array['filter']['announcement_file'] = array('set' => false,
											'chk' => 2,
											'regexp' => '/^['.XIVO_SRE_IPBX_AST_REGEXP_AUDIO_FILE_PATTERN.']'.
								XIVO_SRE_IPBX_AST_REGEXP_AUDIO_FILE_LENGTH.'$/');
$array['filter']['announcement_play'] = array('bool' => true);
$array['filter']['announcement_caller'] = array('bool' => true);
$array['filter']['commented'] = array('bool' => true);
$array['filter']['description'] = array('set' => false,'chk' => 2,'maxlen' => 1000,'cast' => 'sgraph_crlf_tab','eol' => true);

?>
