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
    // Get GITHUB_TOKEN if running on GitHub.
    $token_string = '';
    foreach (getenv() as $settingKey => $settingValue) {
      if ($settingKey == 'GH_TOKEN') {
        $token = $settingValue;
        $token_string = " --github-token=$token";
      }
    }

    global $bee_test_root;
    // Single module.
    $output_single = shell_exec('bee download' . $token_string . ' simplify');
    $pattern = '/\'simplify\' \([\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/backdrop\/modules\/simplify\'/';
    $this->assertMatchesRegularExpression($pattern, $output_single);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/modules/simplify/simplify.info"));

    // Multiple projects (theme and layout).
    $output_multiple = shell_exec('bee download' . $token_string . ' lumi bamboo');
    $pattern = '/\'lumi\' \([\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/backdrop\/themes\/lumi\'/';
    $this->assertMatchesRegularExpression($pattern, $output_multiple);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/themes/lumi/lumi.info"));
    $pattern = '/\'bamboo\' \([\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/backdrop\/layouts\/bamboo\'/';
    $this->assertMatchesRegularExpression($pattern, $output_multiple);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/layouts/bamboo/bamboo.info"));

    // Defined release.
    $output_defined_release = shell_exec('bee download' . $token_string . ' layout_custom_theme:1.x-1.0.4');
    $pattern = '/\'layout_custom_theme\' \(1\.x\-1\.0\.4\, published at 2024\-02\-01T[\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/backdrop\/modules\/layout_custom_theme\'/';
    // Cleanup downloads.
    exec("rm -fr $bee_test_root/backdrop/modules/simplify $bee_test_root/backdrop/themes/lumi $bee_test_root/backdrop/layouts/bamboo $bee_test_root/backdrop/modules/layout_custom_theme");
  }

  /**
   * Make sure that the download-core command works.
   */
  public function test_download_core_command_works() {
    // Get GITHUB_TOKEN if running on GitHub.
    $token_string = '';
    foreach (getenv() as $settingKey => $settingValue) {
      if ($settingKey == 'GH_TOKEN') {
        $token = $settingValue;
        $token_string = " --github-token=$token";
      }
    }

    global $bee_test_root;
    // Download to current directory.
    $output_current = shell_exec("mkdir $bee_test_root/current && cd $bee_test_root/current && bee download-core" . $token_string);
    $pattern = '/Backdrop \([\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/current\'/';
    $this->assertMatchesRegularExpression($pattern, $output_current);
    $this->assertTrue(file_exists("$bee_test_root/current/index.php"));

    // Download to specified directory.
    $output_directory = shell_exec("bee download-core $bee_test_root/directory" . $token_string);
    $pattern = '/Backdrop \([\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/directory\'/';
    $this->assertMatchesRegularExpression($pattern, $output_directory);
    $this->assertTrue(file_exists("$bee_test_root/directory/index.php"));

    // Download a defined release.
    $output_defined_release = shell_exec("bee download-core $bee_test_root/defined_release --version=1.30.0" . $token_string);
    $pattern = '/Backdrop \(1\.30\.0\, published at 2025\-01\-1\dT[\w\s\.\W]*\) was downloaded into \'' . preg_quote($bee_test_root, '/') . '\/defined_release\'/';
    $this->assertMatchesRegularExpression($pattern, $output_defined_release);
    $this->assertTrue(file_exists("$bee_test_root/defined_release/index.php"));

    // Cleanup downloads.
    exec("rm -fr $bee_test_root/current $bee_test_root/directory $bee_test_root/defined_release");
  }

}
