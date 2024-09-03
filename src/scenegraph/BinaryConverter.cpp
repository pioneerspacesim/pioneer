// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
#include "BinaryConverter.h"
#include "FileSystem.h"
#include "GameSaveError.h"
#include "NodeVisitor.h"
#include "Parser.h"
#include "StringF.h"
#include "core/LZ4Format.h"
#include "scenegraph/Animation.h"
#include "scenegraph/Label3D.h"
#include "scenegraph/MatrixTransform.h"
#include "scenegraph/Serializer.h"
#include "scenegraph/Tag.h"
#include "utils.h"

extern "C" {
#include <miniz/miniz.h>
}

using namespace SceneGraph;

union SGM_STRING_VALUE {
	char name[4];
	Uint32 value;
};
const SGM_STRING_VALUE SGM_STRING_ID = { { 's', 'g', 'm', SGM_VERSION } };
const std::string SGM_EXTENSION = ".sgm";
const std::string SAVE_TARGET_DIR = "binarymodels";

class SaveHelperVisitor : public NodeVisitor {
public:
	SaveHelperVisitor(Serializer::Writer *wr, Model *m)
	{
		db.wr = wr;
		db.rd = nullptr;
		db.model = m;
	}

	virtual void ApplyNode(Node &n) override
	{
		n.Save(db);
	}

	virtual void ApplyGroup(Group &g) override
	{
		ApplyNode(static_cast<Node &>(g));
		db.wr->Int32(g.GetNumChildren());
		g.Traverse(*this);
	}

	NodeDatabase db;
};

BinaryConverter::BinaryConverter(Graphics::Renderer *r) :
	BaseLoader(r),
	m_patternsUsed(false)
{
	//register core loaders
	RegisterLoader("Group", &Group::Load);
	RegisterLoader("MatrixTransform", &MatrixTransform::Load);
	RegisterLoader("LOD", &LOD::Load);
	RegisterLoader("StaticGeometry", &StaticGeometry::Load);
	RegisterLoader("CollisionGeometry", &CollisionGeometry::Load);
	RegisterLoader("Thruster", &Thruster::Load);
	RegisterLoader("Label3D", &LoadLabel3D);
	RegisterLoader("Tag", &Tag::Load);
}

void BinaryConverter::RegisterLoader(const std::string &typeName, std::function<Node *(NodeDatabase &)> func)
{
	m_loaders[typeName] = func;
}

void BinaryConverter::Save(const std::string &filename, Model *m)
{
	PROFILE_SCOPED()
	static const std::string s_EmptyString;
	Save(filename, s_EmptyString, m, false);
}

void BinaryConverter::Save(const std::string &filename, const std::string &savepath, Model *m, const bool bInPlace)
{
	PROFILE_SCOPED()
	printf("Saving file (%s)\n", filename.c_str());
	FILE *f = nullptr;
	FileSystem::FileSourceFS newFS(FileSystem::GetDataDir());
	if (!bInPlace) {
		if (!FileSystem::userFiles.MakeDirectory(SAVE_TARGET_DIR))
			throw CouldNotOpenFileException();

		std::string newpath = savepath.substr(0, savepath.size() - filename.size());
		size_t pos = newpath.find_first_of("/", 0);
		while (pos < savepath.size() - filename.size()) {
			newpath = savepath.substr(0, pos);
			pos = savepath.find_first_of("/", pos + 1);
			if (!FileSystem::userFiles.MakeDirectory(FileSystem::JoinPathBelow(SAVE_TARGET_DIR, newpath)))
				throw CouldNotOpenFileException();
			printf("Made directory (%s)\n", FileSystem::JoinPathBelow(SAVE_TARGET_DIR, newpath).c_str());
		}

		f = FileSystem::userFiles.OpenWriteStream(
			FileSystem::JoinPathBelow(SAVE_TARGET_DIR, savepath + SGM_EXTENSION));
		printf("Save file (%s)\n", FileSystem::JoinPathBelow(SAVE_TARGET_DIR, savepath + SGM_EXTENSION).c_str());
		if (!f) throw CouldNotOpenFileException();
	} else {
		f = newFS.OpenWriteStream(savepath + SGM_EXTENSION);
		if (!f) throw CouldNotOpenFileException();
	}

	Serializer::Writer wr;

	wr.Int32(SGM_STRING_ID.value);

	wr.Int32(SGM_VERSION);

	wr.String(m->GetName().c_str());

	SaveMaterials(wr, m);

	SaveHelperVisitor sv(&wr, m);
	m->GetRoot()->Accept(sv);

	m->GetCollisionMesh()->Save(wr);
	wr.Float(m->GetDrawClipRadius());

	SaveAnimations(wr, m);

	//save tags
	wr.Int32(m->GetNumTags());
	for (unsigned int i = 0; i < m->GetNumTags(); i++)
		wr.String(m->GetTagByIndex(i)->GetName().c_str());

	// compress in memory, write to open file
	size_t outSize = 0;
	const std::string &data = wr.GetData();
	try {
		std::string compressedData = lz4::CompressLZ4(data, 6);
		outSize = compressedData.size();
		Output("Compressed model (%s): %.2f KB -> %.2f KB\n", filename.c_str(), data.size() / 1024.f, outSize / 1024.f);
		fwrite(compressedData.data(), outSize, 1, f);
		fclose(f);
	} catch (std::runtime_error &e) {
		Log::Error("Error saving SGM model: {}\n", e.what());
		throw CouldNotWriteToFileException();
	}
}

Model *BinaryConverter::Load(const std::string &filename)
{
	return Load(filename, "models");
}

Model *BinaryConverter::Load(const std::string &name, RefCountedPtr<FileSystem::FileData> binfile)
{
	PROFILE_SCOPED()
	Model *model(nullptr);
	// decompress the loaded ByteRange in memory
	const ByteRange bin = binfile->AsByteRange();
	if (lz4::IsLZ4Format(bin.begin, bin.Size())) {
		try {
			std::string decompressedData = lz4::DecompressLZ4({ bin.begin, bin.Size() });
			// Output("decompressed model file %s (%.2f KB) -> %.2f KB\n", name.c_str(), binfile->GetSize() / 1024.f, decompressedData.size() / 1024.f);
			Serializer::Reader rd(ByteRange(decompressedData.data(), decompressedData.size()));
			model = CreateModel(name, rd);
		} catch (std::runtime_error &e) {
			Log::Error("Error loading SGM model: {}\n", e.what());
		}
	} else {
		void *pDecompressedData;
		size_t outSize(0);
		{
			PROFILE_SCOPED_DESC("tinfl_decompress_mem_to_heap")
			pDecompressedData = tinfl_decompress_mem_to_heap(&bin[0], bin.Size(), &outSize, 0);
		}
		// Output("decompressed model file %s (%.2f KB) -> %.2f KB\n", name.c_str(), binfile->GetSize() / 1024.f, outSize / 1024.f);
		if (pDecompressedData) {
			// now parse in-memory representation as new ByteRange.
			Serializer::Reader rd(ByteRange(static_cast<char *>(pDecompressedData), outSize));
			model = CreateModel(name, rd);
			mz_free(pDecompressedData);
		} else {
			Log::Warning("BinaryConverter failed to load old-style SGM called: {}\n", name.c_str());
		}
	}

	return model;
}

Model *BinaryConverter::Load(const std::string &shortname, const std::string &basepath)
{
	PROFILE_SCOPED()
	FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
	for (FileSystem::FileEnumerator files(fileSource, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();

		//check it's the expected type
		if (info.IsFile() && ends_with_ci(fpath, SGM_EXTENSION)) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();

			if (shortname == name.substr(0, name.length() - SGM_EXTENSION.length())) {
				//curPath is used to find textures, patterns,
				//possibly other data files for this model.
				//Strip trailing slash
				m_curPath = info.GetDir();
				if (m_curPath[m_curPath.length() - 1] == '/')
					m_curPath = m_curPath.substr(0, m_curPath.length() - 1);

				RefCountedPtr<FileSystem::FileData> binfile = info.Read();
				if (binfile.Valid()) return Load(name, binfile);
			}
		}
	}

	throw(LoadingError("File not found"));
	return nullptr;
}

Model *BinaryConverter::CreateModel(const std::string &filename, Serializer::Reader &rd)
{
	PROFILE_SCOPED()
	//verify signature
	const Uint32 sig = rd.Int32();
	if (sig != SGM_STRING_ID.value) { //'SGM#'
		Log::Warning("Error whilst loading {}\nSGM versioning ({}) did not match the supported SGM STRING ID ({})\nSGM file will be ignored\n", filename.c_str(), sig, SGM_STRING_ID.value);
		return nullptr;
	}

	const Uint32 version = rd.Int32();
	if (version != SGM_VERSION) {
		Log::Warning("Error whilst loading {}\nSGM versioning ({}) did not match the supported SGM_VERSION ({})\nSGM file will be ignored\n", filename.c_str(), version, SGM_VERSION);
		return nullptr;
	}

	const std::string modelName = rd.String();

	m_model = new Model(m_renderer, modelName);

	m_patternsUsed = false;
	LoadMaterials(rd);

	Group *root = dynamic_cast<Group *>(LoadNode(rd));
	if (!root) throw LoadingError("Expected root");
	m_model->m_root.Reset(root);

	RefCountedPtr<CollMesh> collMesh(new CollMesh());
	collMesh->Load(rd);
	m_model->SetCollisionMesh(collMesh);
	m_model->SetDrawClipRadius(rd.Float());

	LoadAnimations(rd);

	m_model->InitAnimations();
	//m_model->CreateCollisionMesh();
	if (m_patternsUsed) SetUpPatterns();

	m_model->UpdateTagTransforms();

	return m_model;
}

void BinaryConverter::SaveMaterials(Serializer::Writer &wr, Model *model)
{
	PROFILE_SCOPED()
	//Look for the .model definition and parse it
	//for material definitions
	const ModelDefinition &modelDef = FindModelDefinition(model->GetName());

	wr.Int32(modelDef.matDefs.size());

	for (const auto &m : modelDef.matDefs) {
		wr.String(m.name);
		wr.String(m.tex_diff);
		wr.String(m.tex_spec);
		wr.String(m.tex_glow);
		wr.String(m.tex_ambi);
		wr.String(m.tex_norm);
		wr.Color4UB(m.diffuse);
		wr.Color4UB(m.specular);
		wr.Color4UB(m.ambient);
		wr.Color4UB(m.emissive);
		wr.Int16(m.shininess);
		wr.Int16(m.opacity);
		wr.Bool(m.alpha_test);
		wr.Bool(m.unlit);
		wr.Bool(m.use_pattern);
	}
}

void BinaryConverter::LoadMaterials(Serializer::Reader &rd)
{
	PROFILE_SCOPED()
	for (Uint32 numMats = rd.Int32(); numMats > 0; numMats--) {
		MaterialDefinition m("");
		m.name = rd.String();
		m.tex_diff = rd.String();
		m.tex_spec = rd.String();
		m.tex_glow = rd.String();
		m.tex_ambi = rd.String();
		m.tex_norm = rd.String();
		m.diffuse = rd.Color4UB();
		m.specular = rd.Color4UB();
		m.ambient = rd.Color4UB();
		m.emissive = rd.Color4UB();
		m.shininess = rd.Int16();
		m.opacity = rd.Int16();
		m.alpha_test = rd.Bool();
		m.unlit = rd.Bool();
		m.use_pattern = rd.Bool();

		if (m.use_pattern) m_patternsUsed = true;

		ConvertMaterialDefinition(m);
	}
}

void BinaryConverter::SaveAnimations(Serializer::Writer &wr, Model *m)
{
	PROFILE_SCOPED()
	const auto &anims = m->GetAnimations();
	wr.Int32(anims.size());
	for (const auto &anim : anims) {
		wr.String(anim->GetName());
		wr.Double(anim->GetDuration());
		wr.Int32(anim->GetChannels().size());
		for (const auto &chan : anim->GetChannels()) {
			wr.String(chan.node->GetName());
			//write pos/rot/scale keys
			wr.Int32(chan.positionKeys.size());
			for (const auto &pkey : chan.positionKeys) {
				wr.Double(pkey.time);
				wr.Vector3f(pkey.position);
			}
			wr.Int32(chan.rotationKeys.size());
			for (const auto &rkey : chan.rotationKeys) {
				wr.Double(rkey.time);
				wr.WrQuaternionf(rkey.rotation);
			}
			wr.Int32(chan.scaleKeys.size());
			for (const auto &skey : chan.scaleKeys) {
				wr.Double(skey.time);
				wr.Vector3f(skey.scale);
			}
		}
	}
}

void BinaryConverter::LoadAnimations(Serializer::Reader &rd)
{
	PROFILE_SCOPED()
	//load channels and PRS keys
	const Uint32 numAnims = rd.Int32();
	for (Uint32 i = 0; i < numAnims; i++) {
		const std::string animName = rd.String();
		const double duration = rd.Double();
		Animation *anim = new Animation(animName, duration);
		const Uint32 numChans = rd.Int32();
		for (Uint32 j = 0; j < numChans; j++) {
			const std::string tgtName = rd.String();
			MatrixTransform *tgtNode = dynamic_cast<MatrixTransform *>(m_model->m_root->FindNode(tgtName));
			anim->m_channels.push_back(AnimationChannel(tgtNode));
			auto &chan = anim->m_channels.back();
			for (Uint32 numKeys = rd.Int32(); numKeys > 0; numKeys--) {
				const double ktime = rd.Double();
				const vector3f kpos = rd.Vector3f();
				chan.positionKeys.push_back(PositionKey(ktime, kpos));
			}
			for (Uint32 numKeys = rd.Int32(); numKeys > 0; numKeys--) {
				const double ktime = rd.Double();
				const Quaternionf krot = rd.RdQuaternionf();
				chan.rotationKeys.push_back(RotationKey(ktime, krot));
			}
			for (Uint32 numKeys = rd.Int32(); numKeys > 0; numKeys--) {
				const double ktime = rd.Double();
				const vector3f kscale = rd.Vector3f();
				chan.scaleKeys.push_back(ScaleKey(ktime, kscale));
			}
		}
		m_model->m_animations.push_back(anim);
	}
}

ModelDefinition BinaryConverter::FindModelDefinition(const std::string &shortname)
{
	PROFILE_SCOPED()
	const std::string basepath = "models";

	FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
	for (FileSystem::FileEnumerator files(fileSource, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();

		//check it's the expected type
		if (info.IsFile() && ends_with_ci(fpath, ".model")) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();

			if (shortname == name.substr(0, name.length() - 6)) {
				ModelDefinition modelDefinition;
				try {
					//curPath is used to find textures, patterns,
					//possibly other data files for this model.
					//Strip trailing slash
					m_curPath = info.GetDir();
					assert(!m_curPath.empty());
					if (m_curPath[m_curPath.length() - 1] == '/')
						m_curPath = m_curPath.substr(0, m_curPath.length() - 1);

					Parser p(fileSource, fpath, m_curPath);
					p.Parse(&modelDefinition);
					return modelDefinition;
				} catch (ParseError &err) {
					Output("%s\n", err.what());
					throw LoadingError(err.what());
				}
			}
		}
	}
	throw(LoadingError("File not found"));
}

Node *BinaryConverter::LoadNode(Serializer::Reader &rd)
{
	PROFILE_START()
	const std::string ntype = rd.String();
	const std::string nname = rd.String();
	//Output("Loading: %s %s\n", ntype.c_str(), nname.c_str());
	const Uint32 nmask = rd.Int32();
	const Uint32 nflags = rd.Int32();
	Node *node = nullptr;

	NodeDatabase db;
	db.loader = this;
	db.model = m_model;
	db.rd = &rd;

	auto loadFuncIt = m_loaders.find(ntype);
	if (loadFuncIt == m_loaders.end()) {
		Output("No loader for: %s\n", ntype.c_str());
		return new Group(m_renderer);
	}

	node = loadFuncIt->second(db);

	//register tag nodes
	if (nflags & NODE_TAG)
		m_model->m_tags.push_back(static_cast<Tag *>(node));

	node->SetName(nname);
	node->SetNodeMask(nmask);
	node->SetNodeFlags(nflags);

	// Stop profiling this func before loading children to avoid a giant tree of calls
	PROFILE_STOP()

	if (Group *grp = dynamic_cast<Group *>(node)) {
		Uint32 numChildren = rd.Int32();
		for (Uint32 i = 0; i < numChildren; i++)
			grp->AddChild(LoadNode(rd));
	}

	return node;
}

Label3D *BinaryConverter::LoadLabel3D(NodeDatabase &db)
{
	PROFILE_SCOPED()
	Label3D *lbl = new Label3D(db.loader->GetRenderer(), db.loader->GetLabel3DFont());
	lbl->SetText("NCC-1982");
	return lbl;
}
