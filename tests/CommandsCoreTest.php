<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Core commands.
 */

use PHPUnit\Framework\TestCase;

class CommandsCoreTest extends TestCase {

  /**
   * Check that the Cache Clear command works.
   */
  public function testCacheClear() {
    $output_all = shell_exec('b cc all');
    $this->assertStringContainsString('All cache cleared', $output_all);

    $output_menu = shell_exec('b cc menu');
    $this->assertStringContainsString('Menu cache cleared', $output_menu);

    $output_update = shell_exec('b cc update');
    $this->assertStringContainsString('Update data cache cleared', $output_update);
  }

  /**
   * Check that the Config List command works.
   */
  public function testConfigList() {
    $output = shell_exec('b config-list');
    $this->assertStringContainsString('system.core', $output);
    $this->assertStringContainsString('System core settings', $output);
  }

  /**
   * Check that the Config Get command works.
   */
  public function testConfigGet() {
    $output = shell_exec('b config-get system.core');
    $this->assertStringContainsString('site_favicon_path', $output);
    $this->assertStringContainsString('core/misc/favicon.ico', $output);
  }

  /**
   * Check that the Config Set command works.
   */
  public function testConfigSet() {
    $config = 'system.core';
    $setting = 'site_frontpage';

    // Make sure the old homepage is 'home'.
    exec("b config-get $config", $output_get_old);
    foreach ($output_get_old as $output) {
      if (strpos($output, $setting) !== FALSE) {
        $this->assertStringContainsString('home', $output);
      }
    }

    $output_set = shell_exec("b config-set $config $setting about");
    $this->assertStringContainsString('Config updated', $output_set);

    // Make sure the new homepage is 'about'.
    exec("b config-get $config", $output_get_new);
    foreach ($output_get_new as $output) {
      if (strpos($output, $setting) !== FALSE) {
        $this->assertStringContainsString('about', $output);
      }
    }

    // Set frontpage back to 'home' for future tests.
    exec("b config-set $config $setting home");
  }

  /**
   * Check that the Cron command works.
   */
  public function testCron() {
    $output = shell_exec('b cron');
    $this->assertStringContainsString('Cron processed', $output);
  }

  /**
   * Check that the Watchdog Show command works.
   */
  public function testWatchdogShow() {
    $output_all = shell_exec('b ws');
    $this->assertStringContainsString('Date', $output_all);
    $this->assertStringContainsString('Message', $output_all);

    $output_one = shell_exec('b ws 1');
    $this->assertStringContainsString('ID', $output_one);
    $this->assertStringContainsString('Severity', $output_one);

    exec('b ws --count=2', $output_count);
    $this->assertEquals(3, count($output_count));

    $output_severity = shell_exec('b ws --severity=info');
    $this->assertStringNotContainsString('notice', $output_severity);

    $output_type = shell_exec('b ws --type=cron');
    $this->assertStringNotContainsString('system', $output_type);
  }

  /**
   * Check that the Site Install command works.
   */
  public function testSiteInstall() {
    // Since this test uses environment-specific information, it is only run on
    // Travis CI.
    if (getenv('TRAVIS') !== TRUE) {
      $this->markTestSkipped('This test is only run on Travis CI.');
    }

    $path = getenv('HOME') . '/site_install';
    $db = 'site_install';

    // Download Backdrop and create database.
    exec("git clone https://github.com/backdrop/backdrop.git $path");
    exec("mysql -e 'CREATE DATABASE $db'");

    $output = shell_exec("b si --root=$path --db_url=mysql://travis:@127.0.0.1/$db");
    print_r($output);
    //$this->assertStringContainsString('Cron processed', $output);
  }

}

