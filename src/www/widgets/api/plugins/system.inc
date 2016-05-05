<?php
/*
    Copyright (C) 2016 Deciso B.V.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

function system_api_cpu_stats()
{
    $cpustats = array();
    // take a short snapshot to calculate cpu usage
    $diff = array('user', 'nice', 'sys', 'intr', 'idle');
    $cpuTicks1 = array_combine($diff, explode(" ", get_single_sysctl('kern.cp_time')));
    usleep(100000);
    $cpuTicks2 = array_combine($diff, explode(" ", get_single_sysctl('kern.cp_time')));
    $totalStart = array_sum($cpuTicks1);
    $totalEnd = array_sum($cpuTicks2);
    if ($totalEnd <= $totalStart) {
        // if for some reason the measurement is invalid, assume nothing has changed (all 0)
        $totalEnd = $totalStart;
    }
    $cpustats['used'] = floor(100 * (($totalEnd - $totalStart) - ($cpuTicks2['idle'] - $cpuTicks1['idle'])) / ($totalEnd - $totalStart));
    $cpustats['user'] = floor(100 * (($cpuTicks2['user'] - $cpuTicks1['user'])) / ($totalEnd - $totalStart));
    $cpustats['nice'] = floor(100 * (($cpuTicks2['nice'] - $cpuTicks1['nice'])) / ($totalEnd - $totalStart));
    $cpustats['sys'] = floor(100 * (($cpuTicks2['sys'] - $cpuTicks1['sys'])) / ($totalEnd - $totalStart));
    $cpustats['intr'] = floor(100 * (($cpuTicks2['intr'] - $cpuTicks1['intr'])) / ($totalEnd - $totalStart));
    $cpustats['idle'] = floor(100 * (($cpuTicks2['idle'] - $cpuTicks1['idle'])) / ($totalEnd - $totalStart));

    // cpu model and count
    $cpustats['model'] = get_single_sysctl("hw.model");
    $cpustats['cpus'] = get_single_sysctl('kern.smp.cpus');

    // cpu frequency
    $tmp = get_single_sysctl('dev.cpu.0.freq_levels');
    $cpustats['max.freq'] = !empty($tmp) ? explode("/", explode(" ", $tmp)[0])[0] : "-";
    $tmp = get_single_sysctl('dev.cpu.0.freq');
    $cpustats['cur.freq'] = !empty($tmp) ? $tmp : "-";
    $cpustats['freq_translate'] = sprintf(gettext("Current: %s MHz, Max: %s MHz"), $cpustats['cur.freq'], $cpustats['max.freq']);

    return $cpustats;
}

function system_api_config()
{
    global $config;
    $result = array();
    $result['last_change'] = isset($config['revision']['time']) ? intval($config['revision']['time']) : 0;
    $result['last_change_frmt'] = date("D M j G:i:s T Y", $result['last_change']);
    return $result;
}

function system_api_kernel()
{
    global $config;
    $result = array("pf" => array());
    $result['pf']['maxstates'] = !empty($config['system']['maximumstates']) ? $config['system']['maximumstates'] : default_state_size();
    exec('/sbin/pfctl -si |grep "current entries" 2>/dev/null', $states);
    $result['pf']['states'] = count($states) >  0 ? filter_var($states[0], FILTER_SANITIZE_NUMBER_INT) : 0;
    return $result;
}

/**
 * widget system data
 */
function system_api()
{
    $result = array();

    $result['cpu'] = system_api_cpu_stats();
    preg_match("/sec = (\d+)/", get_single_sysctl("kern.boottime"), $matches);
    $result['uptime'] = time() - $matches[1];
    $result['date_frmt'] = date("D M j G:i:s T Y");
    $result['config'] = system_api_config();
    $result['kernel'] = system_api_kernel();

    return $result;
}