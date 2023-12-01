<?php
/**
 * @file
 * PHPUnit tests for Bee Download commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's command for downloading modules, themes and layouts.
 */
class DownloadCommandsTest extends TestCase {

  /**
   * Make sure that the download command works.
   */
  public function test_download_command_works() {
    global $bee_test_root;
    // Single module.
    $output_single = shell_exec('bee download simplify');
    $this->assertStringContainsString("'simplify' was downloaded into '$bee_test_root/backdrop/modules/simplify'.", (string) $output_single);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/modules/simplify/simplify.info"));

    // Multiple projects (theme and layout).
    $output_multiple = shell_exec('bee download lumi bamboo');
    $this->assertStringContainsString("'lumi' was downloaded into '$bee_test_root/backdrop/themes/lumi'.", (string) $output_multiple);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/themes/lumi/lumi.info"));
    $this->assertStringContainsString("'bamboo' was downloaded into '$bee_test_root/backdrop/layouts/bamboo'.", (string) $output_multiple);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/layouts/bamboo/bamboo.info"));

    // Cleanup downloads.
    exec("rm -fr $bee_test_root/backdrop/modules/simplify $bee_test_root/backdrop/themes/lumi $bee_test_root/backdrop/layouts/bamboo");
  }

  /**
   * Make sure that the download-core command works.
   */
  public function test_download_core_command_works() {
    global $bee_test_root;
    // Download to current directory.
    $output_current = shell_exec("mkdir $bee_test_root/current && cd $bee_test_root/current && bee download-core");
    $this->assertStringContainsString("Backdrop was downloaded into '$bee_test_root/current'.", (string) $output_current);
    $this->assertTrue(file_exists("$bee_test_root/current/index.php"));

    // Download to specified directory.
    $output_directory = shell_exec("bee download-core $bee_test_root/directory");
    $this->assertStringContainsString("Backdrop was downloaded into '$bee_test_root/directory'.", (string) $output_directory);
    $this->assertTrue(file_exists("$bee_test_root/directory/index.php"));

    // Cleanup downloads.
    exec("rm -fr $bee_test_root/current $bee_test_root/directory");
  }

}
