<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output
		method="html"
		encoding="iso-8859-1"
		doctype-public="-//W3C//DTD XHTML 1.1//EN"
		doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
		indent="yes" />

	<!--
	specifies the language of the output, initialize with
	(double hypen)stringparam lang <language>
	We specify a default language to 'en'
	-->
	<xsl:param name="lang">en</xsl:param>

	<!-- this file filename, used for the menu test -->
	<xsl:param name='filename'>index</xsl:param>

	<!-- root element -->
	<xsl:template match="page">
		<!-- create the html basic structure -->
		<html>
			<head>
				<!-- the page title is the content of the title element for the given
				language -->
				<title><xsl:value-of select="title/text[lang($lang)]"/></title>
			</head>
			<body>

				<!-- apply templates -->
				<xsl:apply-templates select="content"/>

				<!-- add the menu at the end -->
				<xsl:call-template name="menu" mode="menu"/>

			</body>
		</html>
	</xsl:template>

	<!-- strip all contents which aren't in the specified language -->
	<xsl:template match="content[not(lang($lang))]"/>

	<!-- process the content -->
	<xsl:template match="content[lang($lang)]">
		<xsl:apply-templates/>
	</xsl:template>

	<!-- each section outputs as a <div> -->
	<xsl:template match="section">
		<div class="section">
			<xsl:apply-templates/>
		</div>
	</xsl:template>

	<!-- define how to generated the menu, which is stored in the menu.xml file -->
	<xsl:template name='menu' mode='menu'>
		<!-- this's a list -->
		<ul id="menu">
			<!-- we define a variable containing all children of <menu> in this file -->
			<xsl:variable name="menu" select="document('menu.xml')/menu"/>
			<!-- we loop through each child -->
			<xsl:for-each select="$menu/item">
				<li class="menuitem">
					<!-- we define it as a link only if we aren't on the same page ! -->
					<xsl:choose>
						<xsl:when test="$filename=@name">
							<xsl:value-of select="./text[@lang=$lang]"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:element name="a">
								<xsl:attribute name="href"><xsl:value-of select="@name"/>.<xsl:value-of select="$lang"/>.html</xsl:attribute>
								<xsl:value-of select="./text[@lang=$lang]"/>
							</xsl:element>
						</xsl:otherwise>
					</xsl:choose>
				</li>
			</xsl:for-each>
		</ul>
	</xsl:template>

	<!-- copy all the rest (attributes) -->
	<xsl:template match="@*">
		<xsl:copy/>
	</xsl:template>

	<!-- copy all the rest (content) -->
	<xsl:template match="*">
		<xsl:copy>
			<xsl:apply-templates select="* | text() | @*"/>
		</xsl:copy>
	</xsl:template>

</xsl:stylesheet>

