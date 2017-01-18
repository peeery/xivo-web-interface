# -*- coding: UTF-8 -*-

# Copyright 2015-2017 The Wazo Authors  (see the AUTHORS file)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

from datetime import datetime

from docker import APIClient as DockerClient
from hamcrest import assert_that, equal_to, has_item, starts_with
from xivo_provd_client import NotFoundError


class ProvdHelper(object):

    DOCKER_PROVD_IMAGE = "wazopbx/xivo-provd"

    DEFAULT_CONFIGS = [{u'X_type': u'registrar',
                        u'deletable': False,
                        u'displayname': u'local',
                        u'id': u'default',
                        u'parent_ids': [],
                        u'proxy_main': u'127.0.0.1',
                        u'raw_config': {u'X_key': u'xivo'},
                        u'registrar_main': u'127.0.0.1',
                        },
                       {u'X_type': u'registrar',
                        u'deletable': True,
                        u'displayname': u'registrar2',
                        u'id': u'registrar2',
                        u'parent_ids': [],
                        u'proxy_main': u'127.0.0.1',
                        u'raw_config': {u'X_key': u'xivo'},
                        u'registrar_main': u'127.0.0.1',
                        },
                       {u'X_type': u'internal',
                        u'deletable': False,
                        u'id': u'autoprov',
                        u'parent_ids': [u'base', u'defaultconfigdevice'],
                        u'raw_config': {u'sccp_call_managers': {u'1': {u'ip': u'127.0.0.1'}},
                                        u'sip_lines': {u'1': {u'display_name': u'Autoprov',
                                                              u'number': u'autoprov',
                                                              u'password': u'autoprov',
                                                              u'proxy_ip': u'127.0.0.1',
                                                              u'registrar_ip': u'127.0.0.1',
                                                              u'username': u'apmy3dCQDw'}}},
                        u'role': u'autocreate'},
                       {u'X_type': u'internal',
                        u'deletable': False,
                        u'id': u'base',
                        u'parent_ids': [],
                        u'raw_config': {u'X_xivo_phonebook_ip': u'127.0.0.1',
                                        u'ntp_enabled': True,
                                        u'ntp_ip': u'127.0.0.1'}},
                       {u'X_type': u'device',
                        u'deletable': False,
                        u'id': u'defaultconfigdevice',
                        u'label': u'Default config device',
                        u'parent_ids': [],
                        u'raw_config': {u'ntp_enabled': True, u'ntp_ip': u'127.0.0.1'}},
                       ]

    def __init__(self, client):
        self.client = client

    @property
    def configs(self):
        return self.client.config_manager()

    @property
    def devices(self):
        return self.client.device_manager()

    def recreate(self):
        self.clean_devices()
        self.clean_configs()
        self.add_default_configs()

    def clean_devices(self):
        for device in self.devices.find():
            self.devices.remove(device['id'])

    def clean_configs(self):
        for config in self.configs.find():
            self.configs.remove(config['id'])

    def add_default_configs(self):
        for config in self.DEFAULT_CONFIGS:
            self.configs.add(config)

    def assert_config_does_not_exist(self, config_id):
        try:
            self.configs.get(config_id)
        except NotFoundError:
            return
        else:
            raise AssertionError('config "{}" exists in xivo-provd'.format(config_id))

    def assert_device_has_autoprov_config(self, device):
        assert_that(device[u'config'], starts_with(u'autoprov'))

    def assert_config_use_device_template(self, config, template_id):
        assert_that(config[u'configdevice'], equal_to(template_id))
        assert_that(config[u'parent_ids'], has_item(template_id))

    def has_synchronized(self, device_id, timestamp=None):
        timestamp = timestamp or datetime.utcnow()
        line = "Synchronizing device {}".format(device_id)
        output = self.find_provd_logs(timestamp)
        for log in output.split("\n"):
            if line in log:
                return True
        return False

    def find_provd_logs(self, timestamp):
        client = DockerClient()
        for container in client.containers(filters={'status': 'running'}):
            info = client.inspect_container(container['Id'])
            if info['Config']['Image'] == self.DOCKER_PROVD_IMAGE:
                return client.logs(container['Id'], since=timestamp)
