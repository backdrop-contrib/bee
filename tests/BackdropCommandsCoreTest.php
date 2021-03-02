<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Core commands.
 */

use PHPUnit\Framework\TestCase;

class BackdropCommandsCoreTest extends TestCase {

  /**
   * Check that the Site Install command works.
   *
   * TODO: @see https://github.com/backdrop-contrib/b/issues/53
   */
  public function testSiteInstall() {
    // TODO: Fix!
    $this->markTestSkipped('This test is still under development...');

    // Since this test uses environment-specific information, it is only run on
    // Travis CI.
    print_r('Travis: ' . getenv('TRAVIS'));
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
  }

  /**
   * Check that the Core Status command works.
   */
  public function testCoreStatus() {
    $asserted = FALSE;

    exec('b st', $output_default);
    foreach ($output_default as $output) {
      if (strpos($output, 'Backdrop root') !== FALSE) {
        $this->assertStringContainsString(getcwd(), $output);
        $asserted = TRUE;
      }
      if (strpos($output, 'Database name') !== FALSE) {
        $this->assertStringContainsString('backdrop', $output);
        $asserted = TRUE;
      }
    }
    $this->assertTrue($asserted, 'Expected output not found.');
  }

  /**
   * Check that the Test callback works.
   */
  public function testTestCallback() {
    $output = shell_exec('b st');
    $this->assertStringContainsString('Backdrop installation detected', $output);
  }

  /**
   * Check that the Update DB Status command works.
   */
  public function testUpdateDbStatus() {
    // Install an old version of Devel.
    exec('cd modules && wget -q https://github.com/backdrop-contrib/devel/releases/download/1.x-1.5.5/devel.zip');
    exec('cd modules && unzip devel.zip && rm devel.zip');
    exec('b en --y devel');

    // Check there are no DB updates.
    $output_clean = shell_exec('b updbst');
    $this->assertStringContainsString('No database updates required', $output_clean);

    // Update Devel to a newer version.
    exec('cd modules && rm -rf devel && wget -q https://github.com/backdrop-contrib/devel/releases/download/1.x-1.6.0/devel.zip');
    exec('cd modules && unzip devel.zip && rm devel.zip');

    // Check for DB updates.
    $output_updates = shell_exec('b updbst');
    $this->assertStringContainsString('Remove option for Krumo skin', $output_updates);
  }

  /**
   * Check that the Update DB command works.
   */
  public function testUpdateDb() {
    // Perform the updates from the previous test.
    $output = shell_exec('b updb --y');
    $this->assertStringContainsString('Remove option for Krumo skin', $output);
    $this->assertStringContainsString('Do you wish to run all pending updates', $output);
    $this->assertStringContainsString('All updates processed', $output);

    // Uninstall Devel for future tests.
    exec('b dis --y devel');
    exec('b pmu --y devel');
    exec('rm -rf modules/devel');
  }

}
